# WinEventsXml

### About

Program is specializes on retrieving Windows events from local and remote machines in XML format.

* Solution is based on [Windows Event Log API](https://docs.microsoft.com/en-us/windows/win32/wes/windows-event-log).
* Handling XML data uses [pugixml library](https://pugixml.org/).

### Requirments

* Windows(Desktop / Server) and RPC support on Target machine
* Windows on launching machine

### Usage

#### Specify parametrs in cmd:

```sh
WinEventsXml.exe [computer] [query_file] [output_xml_file] -m /log [log_file] /bu [backup_file]
```
#### Necessary parameters:
| Command | Description |
| ------ | ------ |
| [computer] | Target machine name/ip. |
| [query_file] | Query file contains [x-path query](https://docs.microsoft.com/en-us/windows/win32/wes/queryschema-schema) for events selection.
| [output_xml_file] | Output file contains result of working in xml file. Result - is set of [events nodes](https://docs.microsoft.com/en-us/windows/win32/wes/eventschema-schema) in <Events/> root node. |

#### Optional parameters:
| Command | Description |
| ------ | ------ |
| -m | Merging mode. Merging events in output file (without repeats). By default file rewrites. |
| /log [log_file] | Write execution log in file. |
| /bu [backup_file] | Backup output file (if exists) before execute. |

### Example

