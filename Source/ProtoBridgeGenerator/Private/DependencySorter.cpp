#include "DependencySorter.h"
#include "GeneratorContext.h"
#include <map>
#include <set>
#include <algorithm>

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4800 4125 4668 4541 4946)
#endif

#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h> 

#ifdef _MSC_VER
#pragma warning(pop)
#endif

namespace
{
	void CollectMessagesRecursive(const google::protobuf::Descriptor* Message, std::vector<const google::protobuf::Descriptor*>& OutMessages)
	{
		if (Message->options().map_entry()) return;

		OutMessages.push_back(Message);
		for (int i = 0; i < Message->nested_type_count(); ++i)
		{
			CollectMessagesRecursive(Message->nested_type(i), OutMessages);
		}
	}

	void CollectMessages(const google::protobuf::FileDescriptor* File, std::vector<const google::protobuf::Descriptor*>& OutMessages)
	{
		for (int i = 0; i < File->message_type_count(); ++i)
		{
			CollectMessagesRecursive(File->message_type(i), OutMessages);
		}
	}

	void BuildDependencyGraph(const std::vector<const google::protobuf::Descriptor*>& Messages, std::map<const google::protobuf::Descriptor*, std::set<const google::protobuf::Descriptor*>>& OutDeps)
	{
		for (const google::protobuf::Descriptor* Msg : Messages)
		{
			for (int i = 0; i < Msg->field_count(); ++i)
			{
				const google::protobuf::FieldDescriptor* Field = Msg->field(i);
				
				if (Field->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
				{
					const google::protobuf::Descriptor* FieldType = Field->message_type();

					if (Field->is_map())
					{
						const google::protobuf::FieldDescriptor* ValField = FieldType->field(1); 
						if (ValField->type() == google::protobuf::FieldDescriptor::TYPE_MESSAGE)
						{
							const google::protobuf::Descriptor* ValType = ValField->message_type();
							if (ValType->file() == Msg->file())
							{
								OutDeps[Msg].insert(ValType);
							}
						}
					}
					else
					{
						if (FieldType->file() == Msg->file() && FieldType != Msg)
						{
							OutDeps[Msg].insert(FieldType);
						}
					}
				}
			}
		}
	}

	void Visit(const google::protobuf::Descriptor* Node, 
					  std::map<const google::protobuf::Descriptor*, std::set<const google::protobuf::Descriptor*>>& Deps,
					  std::set<const google::protobuf::Descriptor*>& Visited,
					  std::set<const google::protobuf::Descriptor*>& TempVisited,
					  std::vector<const google::protobuf::Descriptor*>& Sorted)
	{
		if (TempVisited.count(Node))
		{
			FGeneratorContext::Log("WARNING: Cyclic dependency detected involving " + std::string(Node->full_name()));
			return;
		}

		if (Visited.count(Node)) return;

		TempVisited.insert(Node);

		for (const google::protobuf::Descriptor* Dep : Deps[Node])
		{
			Visit(Dep, Deps, Visited, TempVisited, Sorted);
		}

		TempVisited.erase(Node);
		Visited.insert(Node);
		Sorted.push_back(Node);
	}
}

std::vector<const google::protobuf::Descriptor*> FDependencySorter::Sort(const google::protobuf::FileDescriptor* File)
{
	std::vector<const google::protobuf::Descriptor*> AllMessages;
	CollectMessages(File, AllMessages);

	std::map<const google::protobuf::Descriptor*, std::set<const google::protobuf::Descriptor*>> Deps;
	BuildDependencyGraph(AllMessages, Deps);

	std::vector<const google::protobuf::Descriptor*> Sorted;
	std::set<const google::protobuf::Descriptor*> Visited;
	std::set<const google::protobuf::Descriptor*> TempVisited; 

	for (const google::protobuf::Descriptor* Msg : AllMessages)
	{
		if (Visited.find(Msg) == Visited.end())
		{
			Visit(Msg, Deps, Visited, TempVisited, Sorted);
		}
	}

	return Sorted;
}