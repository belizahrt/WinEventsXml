#include "winevtmerger.h"

namespace wew {

    void XmlNodesInsert(pugi::xml_node& dest, const pugi::xml_node& src)
    {
        for (auto it = src.begin(); it != src.end(); ++it) {
            dest.append_copy(*it);
        }
    }

    WinEvtXmlMerger::WinEvtXmlMerger(const std::wstring& strEvents1,
        const std::wstring& strEvents2)
    {
        pugi::xml_document xmlEvents1;
        LoadFromBuffer(strEvents1, xmlEvents1);

        pugi::xml_document xmlEvents2;
        LoadFromBuffer(strEvents2, xmlEvents2);

        pugi::xml_node outEventsNode = xmlEvents2.child("Events");
        pugi::xml_node inEventsNode = xmlEvents1.child("Events");

        for (auto inEvent : inEventsNode) {
            pugi::xml_node systemNode = inEvent.child("System");
            MergePredicate predicate { 
                systemNode.child("Computer").text().as_string(),
                systemNode.child("EventRecordID").text().as_uint()
            };

            if (outEventsNode.find_child(predicate).empty())
                outEventsNode.append_copy(inEvent);
        }

        pugi::xml_node eventsNode = m_xmlEvents.append_child("Events");
        XmlNodesInsert(eventsNode, outEventsNode);
    }

    pugi::xml_document WinEvtXmlMerger::GetXmlEvents()
    {
        return std::move(m_xmlEvents);
    }

    pugi::xml_parse_result WinEvtXmlMerger::LoadFromBuffer(
        const std::wstring& buffer, pugi::xml_document& destination)
    {
        std::size_t size = buffer.length() * sizeof(wchar_t);
        return destination.load_buffer(buffer.c_str(), size);
    }

}