## Smartnet Scanner

GNURadio based scanner for SmartNet II Digital radio systems.

### Channel List
In order to work, the scanner needs a list of the talkgroups on the system. This is stored in a Comma Sperated Value (CSV) file, that should be name ChanList.csv.

In order to get this data, goto [Radio Reference](http://www.radioreference.com/apps/db/) and find the system. Click on "List All in one table". Select the tabel and copy it into a spreadsheet program like Excel, or Google Docs. It should have 7 different. The value in the collumns should be as follows: 

 `DEC - HEX - Mode - Alpha Tag - Description - Tag` 

Save this file in the CSV format. 

### Installation
**Requirements**
- GNURadio 3.6
- My version of [gr-dsd](https://github.com/robotastic/gr-dsd)

Once all this is installed then
`cmake .
make`






