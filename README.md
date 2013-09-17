## Smartnet Scanner

GNURadio based scanner for SmartNet II Digital radio systems. It is for systems that use P25 CAI for the voice channels instead of analog.

### Channel List
In order to work, the scanner needs a list of the talkgroups on the system. This is stored in a Comma Sperated Value (CSV) file, that should be named ChanList.csv.

In order to get this data:
1. Goto [Radio Reference](http://www.radioreference.com/apps/db/) and find the system.
2. Click on "List All in one table". 
3. Select the tabel and copy it into a spreadsheet program like Excel, or Google Docs.
4. It should have 7 different columns. 
5. The value in the collumns should be as follows: 
 `DEC - HEX - Mode - Alpha Tag - Description - Tag - Group`
6. Save this file in the CSV format. 

### Installation
**Requirements**
- GNURadio 3.6
- My version of [gr-dsd](https://github.com/robotastic/gr-dsd)

Once all this is installed then

	cmake .
	make

### Parameters
Here are the options available for the scanner. I put the parameters I use in _examplestart.sh_.

	./smartnet --freq 856187500.0 --center 857000000.0  --rate 8000000 --error -5500 --rfgain 14 --ifgain 35 --bbgain 35


**--rfgain** _14_

This is the standard gain that you usually set with your SDR program. For the HackRF, it is either on or off. To turn it one, set it to 14.

**--ifgain** _35_

This is an additional gain option that is useful for the HackRF, but may not be set able with SDR. 


**--bbgain** _35_

This is an additional gain option that is useful for the HackRF, but may not be set able with SDR.


**--rate** _4000000_

This is the sample rate that will be used. You want to make sure that the sampling rate you pick is equal to or large than the range of frequencies used by the radio system you are monitoring. Try to make it as small as possible though or else things will be slow because you will have to process lots of extra data. 


**--center** _857000000.0_

This is the center frequency that the SDR will be tuned to. It should be in middle of the lowest frequency and the highest frequency of the system you are trying to monitor. There is usally a spike at the center, so make sure you don't have the center freq landing on one of the channels in the radio system.


**--freq** _856187500.0_

This is the SmartNet control channel frequency for the system you monitoring. This program will listen to it and track the different channel assignments for the talkgroups. Check [Radio Reference](http://www.radioreference.com/apps/db/) to find this frequency.


**--error** _-5500_

SDRs are great, but the tuning is always off by a little unless you have an external clock. This setting lets you correct for this error. The amount of error will generally be different for each device. For my HackRF, I have found the amount of error to be pretty stable though. However it is different at different spots in the sprectrum, so figure it out for each system you are looking at. In order to find yours, fire up a program like  [GQRX](http://gqrx.dk/), tune in the control channel for the system you want to monitor and determine how far it is off from the specified frequency. Note: the frequency tuned in is actuall calculated as: _target - error_. So for with system it was: Target = 856187500.0 and Actual = 85619300.0, so the error was: Target - Actual = -5500.









