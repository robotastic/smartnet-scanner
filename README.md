## Smartnet Scanner

GNURadio based scanner for SmartNet II Digital radio systems. It is for systems that use P25 CAI for the voice channels instead of analog.

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

	cmake .
	make

### Parameters
Here are the options available for the scanner. I put the parameters I use in _examplestart.sh_.

**--rfgain**
This is the standard gain that you usually set with your SDR program. For the HackRF, it is either on or off. To turn it one, set it to 14.
**--ifgain**
This is an additional gain option that is useful for the HackRF, but may not be set able with SDR. 
**--bbgain**
This is an additional gain option that is useful for the HackRF, but may not be set able with SDR. 
**--rate**
This is the sample rate that will be used. You want to make sure that the sampling rate you pick is equal to or large than the range of frequencies used by the 

Once you run the file you will see a large FFT that will display all of the spectrum the SDR sees. You can type in the center frequency into the Frequency box. You want to pick a center frequency that is close to your target frequency, but not the same. This is because there is some DC interference right at the center frequency. In order to tune in your target frequency, you type in the offset into the Xlate Offset box. So if you are trying to tune in 856.8175 Mhz, you could type 856800000 into the Frequency box and 17500 into the Xlating Offset box. (You may actually need to type -17500, I get confused on this and I think the final frequency display is broken). This will tune you to the correct frequency.

Unforunately SDRs are not 100% accruate. Click on the Xlate-1 tab. It will display an FFT of the channel that you are trying to decode. Use Fine Offset slider to center the spike of your channel in the middle. You may need to adjust the Gain up or down to get it to play correctly. The gain slider adjusts the IF & BB gain used in the HackRF. You could tie it to the RF gain instead if you change the OsmoSDR Source block.





