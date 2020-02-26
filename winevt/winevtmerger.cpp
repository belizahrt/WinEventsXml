#include "winevtmerger.h"

namespace wew {

    void XmlNodesInsert(pugi::xml_node& dest, const pugi::xml_node& src)
    {
        for (auto it = src.begin(); it != src.end(); ++it) {
            dest.append_copy(*it);
        }
    }

    std::tuple<pugi::xml_document, std::size_t> 
    WinEvtXmlMerger::Merge()
    {
        pugi::xml_node outEventsNode = m_oXmlEvents.child("Events");
        pugi::xml_node inEventsNode = m_iXmlEvents.child("Events");

        pugi::xml_document mergedXmlEvents;
        pugi::xml_node mergedEventsNode = mergedXmlEvents.append_child("Events");
        XmlNodesInsert(mergedEventsNode, outEventsNode);

        std::size_t eventsInserted {};
        for (auto inEvent : inEventsNode) {
            pugi::xml_node systemNode = inEvent.child("System");
            MergePredicate isExists {
                systemNode.child("Computer").text().as_string(),
                systemNode.child("EventRecordID").text().as_uint()
            };

            if (mergedEventsNode.find_child(isExists).empty()) {
                mergedEventsNode.append_copy(inEvent);
                eventsInserted++;
            }
        }

        return { std::move(mergedXmlEvents), eventsInserted };
    }

    bool WinEvtXmlMerger::setIEventsString(const std::wstring& events)
    {
        if (LoadFromBuffer(events, m_iXmlEvents)) {
            EventsNodeInitialize(m_iXmlEvents);
            return true;
        }
        return false;
    }

    bool WinEvtXmlMerger::setOEventsString(const std::wstring& events)
    {
        if (LoadFromBuffer(events, m_oXmlEvents)) {
            EventsNodeInitialize(m_oXmlEvents);
            return true;
        }
        return false;
    }

    bool WinEvtXmlMerger::loadOEventsFile(const std::wstring& file)
    {
        if (m_oXmlEvents.load_file(file.c_str())) {
            EventsNodeInitialize(m_oXmlEvents);
            return true;
        }
        return false;
    }

    bool HasEvents(pugi::xml_node node)
    {
        std::string nodeName = node.name();

        return nodeName == "Events";
    }

    void  WinEvtXmlMerger::EventsNodeInitialize(pugi::xml_document& document)
    {
        if (document.find_child(HasEvents).empty()) {
            document.append_child("Events");
        }
    }

    pugi::xml_parse_result WinEvtXmlMerger::LoadFromBuffer(
        const std::wstring& buffer, pugi::xml_document& destination)
    {
        std::size_t size = buffer.length() * sizeof(wchar_t);
        return destination.load_buffer(buffer.c_str(), size);
    }

    bool MergePredicate::operator()(pugi::xml_node node) const
    {
        pugi::xml_node systemNode = node.child("System");
        std::string computer =
            systemNode.child("Computer").text().as_string();
        unsigned int recordId =
            systemNode.child("EventRecordID").text().as_uint();

        bool result = (m_Computer == computer && m_recordId == recordId);

        return result;
    }

}