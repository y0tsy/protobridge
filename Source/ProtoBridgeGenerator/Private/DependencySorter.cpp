#include "DependencySorter.h"
#include <map>
#include <set>
#include <stack>
#include <stdexcept>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace
{
	enum class EVisitState
	{
		Unvisited,
		Visiting,
		Visited
	};

	struct FGraphNode
	{
		const google::protobuf::Descriptor* Message;
		std::set<const google::protobuf::Descriptor*> Dependencies;
		EVisitState State = EVisitState::Unvisited;
	};

	void CollectMessages(const google::protobuf::Descriptor* Message, std::vector<const google::protobuf::Descriptor*>& OutMessages)
	{
		if (Message->options().map_entry()) return;

		OutMessages.push_back(Message);
		for (int i = 0; i < Message->nested_type_count(); ++i)
		{
			CollectMessages(Message->nested_type(i), OutMessages);
		}
	}

	void AddDependency(std::map<const google::protobuf::Descriptor*, FGraphNode>& Graph, 
		const google::protobuf::Descriptor* Source, 
		const google::protobuf::Descriptor* Target)
	{
		if (Source == Target) return; 
		if (Target->file() != Source->file()) return; 

		Graph[Source].Dependencies.insert(Target);
	}

	void BuildGraph(const std::vector<const google::protobuf::Descriptor*>& Messages, std::map<const google::protobuf::Descriptor*, FGraphNode>& OutGraph)
	{
		for (const google::protobuf::Descriptor* Msg : Messages)
		{
			OutGraph[Msg].Message = Msg;
		}

		for (const google::protobuf::Descriptor* Msg : Messages)
		{
			for (int i = 0; i < Msg->field_count(); ++i)
			{
				const google::protobuf::FieldDescriptor* Field = Msg->field(i);

				if (Field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
				{
					const google::protobuf::Descriptor* Dependency = Field->message_type();
					
					if (Field->is_map())
					{
						const google::protobuf::FieldDescriptor* ValueField = Dependency->field(1);
						if (ValueField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
						{
							AddDependency(OutGraph, Msg, ValueField->message_type());
						}
					}
					else
					{
						AddDependency(OutGraph, Msg, Dependency);
					}
				}
			}

			for (int i = 0; i < Msg->oneof_decl_count(); ++i)
			{
				const google::protobuf::OneofDescriptor* OneOf = Msg->oneof_decl(i);
				for (int j = 0; j < OneOf->field_count(); ++j)
				{
					const google::protobuf::FieldDescriptor* Field = OneOf->field(j);
					if (Field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
					{
						AddDependency(OutGraph, Msg, Field->message_type());
					}
				}
			}
		}
	}

	void TopologicalSort(std::map<const google::protobuf::Descriptor*, FGraphNode>& Graph, std::vector<const google::protobuf::Descriptor*>& OutSorted)
	{
		std::stack<const google::protobuf::Descriptor*> Stack;
		
		for (auto& Pair : Graph)
		{
			if (Pair.second.State == EVisitState::Unvisited)
			{
				Stack.push(Pair.first);
				
				while (!Stack.empty())
				{
					const google::protobuf::Descriptor* Current = Stack.top();
					FGraphNode& Node = Graph[Current];

					if (Node.State == EVisitState::Unvisited)
					{
						Node.State = EVisitState::Visiting;
						for (const google::protobuf::Descriptor* Dep : Node.Dependencies)
						{
							FGraphNode& DepNode = Graph[Dep];
							if (DepNode.State == EVisitState::Unvisited)
							{
								Stack.push(Dep);
							}
							else if (DepNode.State == EVisitState::Visiting)
							{
								throw std::runtime_error("Cyclic dependency detected involving " + std::string(Current->full_name()) + " and " + std::string(Dep->full_name()) + ". Unreal Engine USTRUCTs cannot have cyclic dependencies.");
							}
						}
					}
					else
					{
						Stack.pop();
						if (Node.State == EVisitState::Visiting)
						{
							Node.State = EVisitState::Visited;
							OutSorted.push_back(Current);
						}
					}
				}
			}
		}
	}
}

std::vector<const google::protobuf::Descriptor*> FDependencySorter::Sort(const google::protobuf::FileDescriptor* File)
{
	std::vector<const google::protobuf::Descriptor*> AllMessages;
	for (int i = 0; i < File->message_type_count(); ++i)
	{
		CollectMessages(File->message_type(i), AllMessages);
	}

	std::map<const google::protobuf::Descriptor*, FGraphNode> Graph;
	BuildGraph(AllMessages, Graph);

	std::vector<const google::protobuf::Descriptor*> Result;
	TopologicalSort(Graph, Result);

	return Result;
}