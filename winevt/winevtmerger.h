#ifndef WINEVTMERGER_H
#define WINEVTMERGER_H

#include "pugixml/pugixml.hpp"

namespace wew {

    class WinEvtXmlMerger final {
    public:
        WinEvtXmlMerger() = delete;
        WinEvtXmlMerger operator=(const WinEvtXmlMerger&) = delete;
        WinEvtXmlMerger(const WinEvtXmlMerger&) = delete;

        WinEvtXmlMerger(const std::wstring& strEvents1, 
            const std::wstring& strEvents2);

        pugi::xml_document GetXmlEvents();

    private:
        pugi::xml_document m_xmlEvents;

        void EventsNodeInitialize(pugi::xml_document& document);
        pugi::xml_parse_result LoadFromBuffer(const std::wstring& buffer,
            pugi::xml_document& destination);
    };

    struct MergePredicate {
        std::string m_Computer;
        unsigned int m_recordId;

        bool operator()(pugi::xml_node node) const
        {
            pugi::xml_node systemNode = node.child("System");
            std::string computer = 
                systemNode.child("Computer").text().as_string();
            unsigned int recordId = 
                systemNode.child("EventRecordID").text().as_uint();

            bool result = (m_Computer == computer && m_recordId == recordId);

            return result;
        }
    };

}
#endif
