#pragma once

#include "GeneratorContext.h"
#include <vector>
#include <map>
#include <set>
#include <algorithm>

class FDependencySorter
{
public:
    static std::vector<const Descriptor*> Sort(const FileDescriptor* File)
    {
        std::vector<const Descriptor*> AllMessages;
        CollectMessages(File, AllMessages);

        std::map<const Descriptor*, std::set<const Descriptor*>> Deps;
        BuildDependencyGraph(AllMessages, Deps);

        std::vector<const Descriptor*> Sorted;
        std::set<const Descriptor*> Visited;
        std::set<const Descriptor*> TempVisited; 

        for (const Descriptor* Msg : AllMessages)
        {
            if (Visited.find(Msg) == Visited.end())
            {
                Visit(Msg, Deps, Visited, TempVisited, Sorted);
            }
        }

        return Sorted;
    }

private:
    static void CollectMessages(const FileDescriptor* File, std::vector<const Descriptor*>& OutMessages)
    {
        for (int i = 0; i < File->message_type_count(); ++i)
        {
            CollectMessagesRecursive(File->message_type(i), OutMessages);
        }
    }

    static void CollectMessagesRecursive(const Descriptor* Message, std::vector<const Descriptor*>& OutMessages)
    {
        if (Message->options().map_entry()) return;

        OutMessages.push_back(Message);
        for (int i = 0; i < Message->nested_type_count(); ++i)
        {
            CollectMessagesRecursive(Message->nested_type(i), OutMessages);
        }
    }

    static void BuildDependencyGraph(const std::vector<const Descriptor*>& Messages, std::map<const Descriptor*, std::set<const Descriptor*>>& OutDeps)
    {
        for (const Descriptor* Msg : Messages)
        {
            for (int i = 0; i < Msg->field_count(); ++i)
            {
                const FieldDescriptor* Field = Msg->field(i);
                
                if (Field->type() == FieldDescriptor::TYPE_MESSAGE)
                {
                    const Descriptor* FieldType = Field->message_type();

                    if (Field->is_map())
                    {
                        const FieldDescriptor* ValField = FieldType->field(1); 
                        if (ValField->type() == FieldDescriptor::TYPE_MESSAGE)
                        {
                            const Descriptor* ValType = ValField->message_type();
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

    static void Visit(const Descriptor* Node, 
                      std::map<const Descriptor*, std::set<const Descriptor*>>& Deps,
                      std::set<const Descriptor*>& Visited,
                      std::set<const Descriptor*>& TempVisited,
                      std::vector<const Descriptor*>& Sorted)
    {
        if (TempVisited.count(Node))
        {
            FGeneratorContext::Log("WARNING: Cyclic dependency detected involving " + std::string(Node->full_name()));
            return;
        }

        if (Visited.count(Node)) return;

        TempVisited.insert(Node);

        for (const Descriptor* Dep : Deps[Node])
        {
            Visit(Dep, Deps, Visited, TempVisited, Sorted);
        }

        TempVisited.erase(Node);
        Visited.insert(Node);
        Sorted.push_back(Node);
    }
};