#ifndef WINEVTMERGER_H
#define WINEVTMERGER_H

#include "pugixml/pugixml.hpp"

namespace wew {

    class WinEvtXmlMerger final {
    public:
        WinEvtXmlMerger() = default;
        WinEvtXmlMerger operator=(const WinEvtXmlMerger&) = delete;
        WinEvtXmlMerger(const WinEvtXmlMerger&) = delete;

        std::tuple<pugi::xml_document, std::size_t> Merge();

        bool setIEventsString(const std::wstring& events);
        bool setOEventsString(const std::wstring& events);

        bool loadOEventsFile(const std::wstring& file);

    private:
        pugi::xml_document m_iXmlEvents;
        pugi::xml_document m_oXmlEvents;

        void EventsNodeInitialize(pugi::xml_document& document);
        pugi::xml_parse_result LoadFromBuffer(const std::wstring& buffer,
            pugi::xml_document& destination);
    };

    struct MergePredicate {
        bool operator()(pugi::xml_node node) const;

        std::string m_Computer;
        unsigned int m_recordId;
    };

}
#endif
