#include "CMicrocodeUpdater.h"

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <termios.h>

extern "C"
{
	void* StartSessionThread(void* arg)
	{
		CMicrocodeUpdater* pMicrocodeUpdater = static_cast<CMicrocodeUpdater*>(arg);
		pMicrocodeUpdater->StartUpdaterSession();
		return 0;
	}
}

CMicrocodeUpdater::CMicrocodeUpdater()
	: m_vecHexImage(MaxFlashSize, 0xFF)
	, m_vecPageSpecified512(MaxFlashSize / 512, false)
	, m_vecReceiveBuffer(ReceiveBufferSize)
	, m_vecTransmitBuffer(TransmitBufferSize)
{
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("GetHexImageInfo", 0x80, 1, 14));
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("GetPageInfo", 0x81, 1, 6));
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("GetPage", 0x82, 1, 514));
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("DisplayTargetInfo", 0x83, 19, 1));
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("DisplayInfoCode", 0x84, 2, 1));
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("GetSeqFlashBytes", 0x11, 2, TransmitBufferSize));
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("GetRunningCRC", 0x12, 1, 2));
	m_vecDScommand.push_back(make_shared<DataSourceCommand>("ClosePort", 0x13, 2, 0));
}

CMicrocodeUpdater::~CMicrocodeUpdater()
{
	ClosePort();
}

bool CMicrocodeUpdater::OpenHexFile(string& strFilename)
{
	bool retval(false);

	if(m_InputFileStream.is_open())
	{
		m_InputFileStream.close();
	}
	
	m_InputFileStream.open(strFilename.c_str(), std::ifstream::in);

	if(!m_InputFileStream.is_open())
	{
		printf("OpenHexFile() - FAILED to open file: %s\n", strFilename.c_str());
		return false;
	}

	m_strInputFileName = strFilename;

	m_InputFileStream.seekg(0, m_InputFileStream.end);
	m_lFileLength = m_InputFileStream.tellg();
	m_InputFileStream.seekg(0, m_InputFileStream.beg);

	InitHexImage();

	if(ReadHexFile(m_InputFileStream) == true)
	{
		m_bHexImageValid = true;

		// The first byte of AppFW space has to be the lowest specified addr in the hex file
		m_uiAppFWstartAddr = m_uiLowestSpecifiedAddr;

		// Add number of signature/CRC bytes to highest specified addr to get app FW end addr
		m_uiAppFWendAddr = m_uiHighestSpecifiedAddr;

		InitHexImageInfo(m_vecHexImage);

		retval = true;
	}
	else
	{
		printf("\nOpenHexFile() - ERROR: Hex Image is Invalid\n");
	}

	if(m_InputFileStream.is_open())
	{
		m_InputFileStream.close();
	}

	return retval;
}

void CMicrocodeUpdater::InitHexImage()
{
	std::fill(m_vecHexImage.begin(), m_vecHexImage.end(), 0xFF);
	std::fill(m_vecPageSpecified512.begin(), m_vecPageSpecified512.end(), false);

	m_uiLowestSpecifiedAddr = 0xFFFFFFFF;
	m_uiHighestSpecifiedAddr = 0;

	m_bHexImageValid = false;
}

bool CMicrocodeUpdater::StartUpdating(bool bAsync)
{
	bool bResult(true);

	if(bAsync)
	{
		// Create worker thread to run the session thread
		int rc = pthread_create(&m_sessionThreadId, (pthread_attr_t*)0, &StartSessionThread, this);
		if(rc < 0)
		{
			printf("StartUpdating(EXITING) - pthread_create() FAILED\n");
			return false;
		}
	}
	else
	{
		bResult = StartUpdaterSession();
	}

	return bResult;
}

bool CMicrocodeUpdater::StartUpdaterSession()
{
	if(m_fd < 0)
	{
		printf("StartUpdaterSession() - FAILED, m_fd < 0\n");
		return false;
	}

	bool bReturn(true);

	unsigned int uiCounter(0);
	do
	{
		m_vecReceiveBuffer.clear();

		int nBytesRead = read(m_fd, &m_ReadBuffer[0], m_nTotalBytesToReceive);
		if(nBytesRead > 0)
		{
			uiCounter = 0;
			bool bValidCommand = ValidateCommandCode(m_ReadBuffer[0]);
			if(bValidCommand)
			{
				if(m_nTotalBytesToReceive)
				{
					int nMoreBytesRead(0);
					while(nMoreBytesRead < m_nTotalBytesToReceive)
					{
						nMoreBytesRead += read(m_fd, &m_ReadBuffer[1 + nMoreBytesRead], m_nTotalBytesToReceive - nMoreBytesRead);
					}
					nBytesRead += nMoreBytesRead;
				}
				m_nTotalBytesToReceive = 1;

				unsigned char* pData = &m_ReadBuffer[0];
				m_vecReceiveBuffer.assign(pData, pData + nBytesRead);

				SerialPortDataReceived(m_vecReceiveBuffer);
			}
		}
		else if(nBytesRead < 0)
		{
			printf("StartUpdaterSession(EXITING) - read() FAILED, nBytesRead: %d\n", nBytesRead);
			return false;
		}
		else
		{
			if(++uiCounter >= 4)
			{
				break;
			}
		}
	}
	while(!m_bUpdateComplete);

	return m_bUpdateSuccessful;
}

bool CMicrocodeUpdater::ValidateCommandCode(unsigned char rxbyte)
{
	static unsigned int uiErrorCount(0);

	int i = 0;
	bool bCommandOK(false);

	// Iterate through all the commands for a match
	for(unsigned int i = 0; i < m_vecDScommand.size(); i++)
	{
		if(rxbyte == m_vecDScommand[i]->ucCommandCode)
		{
			bCommandOK = true;
			m_nCommandCode = rxbyte;
			m_strCommandName = m_vecDScommand[i]->strCommandName;
			m_nTotalBytesToReceive = m_vecDScommand[i]->nCommandRxSize - 1;

			uiErrorCount = 0;

			break;
		}
	}

	if(!bCommandOK)
	{
		printf("\nValidateCommandCode() - Unknown Command: 0x%02X", rxbyte);

		if(++uiErrorCount >= 12)
		{
			printf("\nValidateCommandCode() - Exiting after 12 unknown commands\n");
			m_bUpdateComplete = true;
		}
	}

	return bCommandOK;
}

void CMicrocodeUpdater::SerialPortDataReceived(std::vector<unsigned char>& vecReceiveBuffer)
{
	unsigned int uiCount = vecReceiveBuffer.size();

	if(uiCount)
	{
		printf("\nSerialPortDataReceived() - uiCount: %u, ", uiCount);

		printf("Bytes: ");
		for(unsigned int i = 0; i < uiCount; i++)
		{
			if(i > 0)
			{
				printf("-");
			}
			printf("%.2X", vecReceiveBuffer[i]);
		}
		printf("\n");
	}

	ProcessCommand();
}

void CMicrocodeUpdater::WriteToSerialPort(unsigned char* pData, int numBytes)
{
	printf("\nWriteToSerialPort() - numBytes: %d, ", numBytes);

	printf("Bytes: ");
	for(int i = 0; i < numBytes; i++)
	{
		if(i > 0)
		{
			printf("-");
		}
		printf("%02X", *(pData + i));
	}
	printf("\n");

	write(m_fd, pData, numBytes);
}

bool CMicrocodeUpdater::ReadHexFile(ifstream& fs)
{
	string rawLine;

	int line_index(0);
	unsigned char running_checksum;

	unsigned char rec_len;
	unsigned char addr_temp;
	unsigned int addr_offset;
	unsigned char rec_type;
	unsigned char data_byte;
	unsigned char line_checksum;

	bool img_parse_success(false);
	bool parse_error(false);

	while(!fs.eof() && (img_parse_success == false) && (parse_error == false))
	{
		/*
		 Intel Hex File Line Format
			:10246200464C5549442050524F46494C4500464C33
			|||||||||||                              CC->Checksum
			|||||||||DD->Data
			|||||||TT->Record Type
			|||AAAA->Address
			|LL->Record Length
			:->Colon
		*/

		rawLine.clear();
		std::getline(fs, rawLine);

		line_index = 0;
		if(rawLine.substr(line_index++, 1) == ":")
		{
			running_checksum = 0;

			rec_len = std::stoi(rawLine.substr(line_index, 2), nullptr, 16);
			line_index += 2;
			running_checksum += rec_len;

			addr_temp = std::stoi(rawLine.substr(line_index, 2), nullptr, 16);
			running_checksum += addr_temp;

			addr_temp = std::stoi(rawLine.substr(line_index + 2, 2), nullptr, 16);
			running_checksum += addr_temp;

			addr_offset = (unsigned int)std::stoi(rawLine.substr(line_index, 4), nullptr, 16);
			line_index += 4;

			rec_type = std::stoi(rawLine.substr(line_index, 2), nullptr, 16);
			line_index += 2;
			running_checksum += rec_type;

			switch(rec_type)
			{
				case 0x00:  // Data Record
				{
					// Loop through the data and store it in our data array that maps to memory
					for(unsigned int data_index = 0; data_index < rec_len; data_index++)
					{
						data_byte = std::stoi(rawLine.substr(line_index, 2), nullptr, 16);
						line_index += 2;

						unsigned int addr = addr_offset + data_index;

						if(addr >= MaxFlashSize)
						{
							printf("\nHex file read error: Addr from hex file out of bounds\n\n");
							parse_error = true;
							break;
						}
						else
						{
							if(addr < m_uiLowestSpecifiedAddr)
							{
								m_uiLowestSpecifiedAddr = addr;
							}
							if(addr > m_uiHighestSpecifiedAddr)
							{
								m_uiHighestSpecifiedAddr = addr;
							}

							m_vecHexImage[addr] = data_byte;
							m_vecPageSpecified512[addr >> 9] = true;
							running_checksum += data_byte;
						}
					}

					if(parse_error)
					{
						break;
					}

					// Read the line checksum
					line_checksum = std::stoi(rawLine.substr(line_index, 2), nullptr, 16);
					running_checksum += line_checksum;

					// If the final sum isn't 0, there was an error
					if(running_checksum != 0x00)
					{
						printf("ReadHexFile() - parse_error, running_checksum (0x%.2X) != 0x00\n", running_checksum);
						parse_error = true;
						break;
					}
					break;
				}
				case 0x01:  // End-of-File Record
				{
					//printf("ReadHexFile() - img_parse_success, End-of-File Record\n");
					img_parse_success = true;
					break;
				}
				default:    // Unknown record type
				{
					printf("ReadHexFile() - parse_error, Unknown record type\n");
					parse_error = true;
					break;
				}
			}

			if(parse_error)
			{
				break;
			}
		}
		else
		{
			printf("ReadHexFile() - substr() FAILED\n");
			parse_error = true;
			break;
		}
	}

	fs.close();

	if((img_parse_success == true) && (parse_error == false))
	{
		return true;
	}
	else
	{
		printf("ReadHexFile(EXITING) - FAILED\n");
		return false;
	}
}

void CMicrocodeUpdater::InitHexImageInfo(std::vector<unsigned char>& hexImage)
{
	m_ucInfoBlockLength = hexImage[m_uiAppFWendAddr - 4];
	m_ucMCUcode = hexImage[m_uiAppFWendAddr - 5];
	m_ucBLtype = hexImage[m_uiAppFWendAddr - 6];
	m_ucFlashPageSizeCode = hexImage[m_uiAppFWendAddr - 7];
	m_ucAppFWversionLow = hexImage[m_uiAppFWendAddr - 8];
	m_ucAppFWversionHigh = hexImage[m_uiAppFWendAddr - 9];
	m_ucCANdeviceAddr = hexImage[m_uiAppFWendAddr - 10];
}

void CMicrocodeUpdater::InitTargetInfo(std::vector<unsigned char>& targetInfoResponse)
{
	unsigned int uiSavedAppFWstartAddr = m_uiAppFWstartAddr;
	unsigned int uiSavedAppFWendAddr = m_uiAppFWendAddr;

	m_ucInfoBlockLength = targetInfoResponse[1];
	m_ucBLfwVersionLow = targetInfoResponse[2];
	m_ucBLfwVersionHigh = targetInfoResponse[3];
	m_ucMCUcode = targetInfoResponse[4];
	m_ucBLtype = targetInfoResponse[5];
	m_ucFlashPageSizeCode = targetInfoResponse[6];
	m_ucBLbufferSizeCode = targetInfoResponse[7];
	m_ucCRCtype = targetInfoResponse[8];
	m_ucEncryptionType = targetInfoResponse[9];
	m_uiAppFWstartAddr = (unsigned int)((targetInfoResponse[10]) | (targetInfoResponse[11] << 8) | (targetInfoResponse[12] << 16));
	m_uiAppFWendAddr = (unsigned int)((targetInfoResponse[13]) | (targetInfoResponse[14] << 8) | (targetInfoResponse[15] << 16));
	m_ucCANdeviceAddr = targetInfoResponse[16];
	m_ucAppFWversionLow = targetInfoResponse[17];
	m_ucAppFWversionHigh = targetInfoResponse[18];

#ifdef DEBUG
	printf("\nInitTargetInfo() - InfoBlockLength: 0x%.2X (%u)\n", m_ucInfoBlockLength, (unsigned int)m_ucInfoBlockLength);
	printf("InitTargetInfo() - BLfwVersionLow: 0x%.2X (%u)\n", m_ucBLfwVersionLow, (unsigned int)m_ucBLfwVersionLow);
	printf("InitTargetInfo() - BLfwVersionHigh: 0x%.2X (%u)\n", m_ucBLfwVersionHigh, (unsigned int)m_ucBLfwVersionHigh);
	printf("InitTargetInfo() - MCUcode: 0x%.2X (%u)\n", m_ucMCUcode, (unsigned int)m_ucMCUcode);
	printf("InitTargetInfo() - BLtype: 0x%.2X (%u)\n", m_ucBLtype, (unsigned int)m_ucBLtype);
	printf("InitTargetInfo() - FlashPageSizeCode: 0x%.2X (%u)\n", m_ucFlashPageSizeCode, (unsigned int)m_ucFlashPageSizeCode);
	printf("InitTargetInfo() - BLbufferSizeCode: 0x%.2X (%u)\n", m_ucBLbufferSizeCode, (unsigned int)m_ucBLbufferSizeCode);
	printf("InitTargetInfo() - CRCtype: 0x%.2X (%u)\n", m_ucCRCtype, (unsigned int)m_ucCRCtype);
	printf("InitTargetInfo() - EncryptionType: 0x%.2X (%u)\n", m_ucEncryptionType, (unsigned int)m_ucEncryptionType);
	printf("InitTargetInfo() - AppFWstartAddr: 0x%.4X (%u)\n", m_uiAppFWstartAddr, (unsigned int)m_uiAppFWstartAddr);
	printf("InitTargetInfo() - AppFWendAddr: 0x%.4X (%u)\n", m_uiAppFWendAddr, (unsigned int)m_uiAppFWendAddr);
	printf("InitTargetInfo() - CANdeviceAddr: 0x%.2X (%u)\n", m_ucCANdeviceAddr, (unsigned int)m_ucCANdeviceAddr);
	printf("InitTargetInfo() - AppFWversionLow: 0x%.2X (%u)\n", m_ucAppFWversionLow, (unsigned int)m_ucAppFWversionLow);
	printf("InitTargetInfo() - AppFWversionHigh: 0x%.2X (%u)\n", m_ucAppFWversionHigh, (unsigned int)m_ucAppFWversionHigh);
#endif

	if(uiSavedAppFWstartAddr != m_uiAppFWstartAddr)
	{
		printf("\nInitTargetInfo() - AppFWstartAddr: 0x%.4X from Hex file does not match AppFWstartAddr: 0x%.4X from used TargetInfo data.\n", uiSavedAppFWstartAddr, m_uiAppFWstartAddr);
	}

	if(uiSavedAppFWendAddr != m_uiAppFWendAddr)
	{
		printf("\nInitTargetInfo() - AppFWendAddr: 0x%.4X from Hex file does not match AppFWendAddr: 0x%.4X from used TargetInfo data.\n", uiSavedAppFWendAddr, m_uiAppFWendAddr);
	}

	return;
}

void CMicrocodeUpdater::LoadHexImageInfoResponse(std::vector<unsigned char>& buf)
{
	buf[0] = SRC_RSP_OK;
	buf[1] = 0x0E;
	buf[2] = m_ucMCUcode;
	buf[3] = m_ucBLtype;
	buf[4] = m_ucFlashPageSizeCode;
	buf[5] = 0xA5;		// flash key 0
	buf[6] = 0xF1;		// flash key 1
	buf[7] = m_ucCANdeviceAddr;
	buf[8] = (unsigned char)(m_uiAppFWstartAddr & 0xFF);
	buf[9] = (unsigned char)((m_uiAppFWstartAddr & 0xFF00) >> 8);
	buf[10] = (unsigned char)((m_uiAppFWstartAddr & 0xFF0000) >> 16);
	buf[11] = (unsigned char)(m_uiAppFWendAddr & 0xFF);
	buf[12] = (unsigned char)((m_uiAppFWendAddr & 0xFF00) >> 8);
	buf[13] = (unsigned char)((m_uiAppFWendAddr & 0xFF0000) >> 16);
}

void CMicrocodeUpdater::ProcessCommand()
{
	if(m_nCommandCode == m_vecDScommand[DScommandSeq::DisplayTargetInfo]->ucCommandCode)
	{
#ifdef DEBUG
		printf("ProcessCommand(DisplayTargetInfo)\n");
#endif

		InitTargetInfo(m_vecReceiveBuffer);

		m_vecTransmitBuffer[0] = SRC_RSP_OK;

		WriteToSerialPort(&m_vecTransmitBuffer[0], m_vecDScommand[DScommandSeq::DisplayTargetInfo]->nCommandTxSize);
	}
	else if(m_nCommandCode == m_vecDScommand[DScommandSeq::DisplayInfoCode]->ucCommandCode)
	{
#ifdef DEBUG
		printf("ProcessCommand(DisplayInfoCode)\n");
#endif

		m_vecTransmitBuffer[0] = SRC_RSP_OK;

		WriteToSerialPort(&m_vecTransmitBuffer[0], m_vecDScommand[DScommandSeq::DisplayInfoCode]->nCommandTxSize);
	}
	else if(m_nCommandCode == m_vecDScommand[DScommandSeq::GetHexImageInfo]->ucCommandCode)
	{
#ifdef DEBUG
		printf("ProcessCommand(GetHexImageInfo)\n");
#endif

		bool bUpdateComplete(false);

		m_vecTransmitBuffer.clear();

		if(m_bHexImageValid == true)
		{
			LoadHexImageInfoResponse(m_vecTransmitBuffer);

			// Reset internal addr and crc variables
			m_uiCurrentAddr = m_uiAppFWstartAddr;
			m_usCurrentCRC = 0;

			// Reset internal page index
			m_nCurrentPageIndex = 0;
			m_nValidPageCount = 0;
			m_vecTransmitBuffer[5] = 0xA5; // Flash key 0
			m_vecTransmitBuffer[6] = 0xF1; // Flash key 1
		}
		else
		{
			m_vecTransmitBuffer[0] = SRC_RSP_ERROR;
			bUpdateComplete = true;
		}

		WriteToSerialPort(&m_vecTransmitBuffer[0], m_vecDScommand[DScommandSeq::GetHexImageInfo]->nCommandTxSize);

		m_bUpdateComplete = bUpdateComplete;
	}
	else if(m_nCommandCode == m_vecDScommand[DScommandSeq::GetPageInfo]->ucCommandCode)
	{
#ifdef DEBUG
		printf("ProcessCommand(GetPageInfo)\n");
#endif

		bool bUpdateComplete(false);

		if(m_bHexImageValid == true)
		{
			do
			{
				bool tmp = m_vecPageSpecified512[m_nCurrentPageIndex];
				if(tmp == true)
				{
					break;
				}
				else
				{
					m_nCurrentPageIndex++;
				}
			}
			while(static_cast<unsigned int>(CurrentPageStartAddr()) <= m_uiAppFWendAddr);

			if(static_cast<unsigned int>(CurrentPageStartAddr()) <= m_uiAppFWendAddr)
			{
				m_vecTransmitBuffer[0] = SRC_RSP_OK;
				m_vecTransmitBuffer[1] = (unsigned char)(CurrentPageStartAddr() & 0xFF);
				m_vecTransmitBuffer[2] = (unsigned char)((CurrentPageStartAddr() & 0xFF00) >> 8);
				m_vecTransmitBuffer[3] = (unsigned char)((CurrentPageStartAddr() & 0xFF0000) >> 16);

				unsigned short crc = GetHexImagePartialCRC(CurrentPageStartAddr(), 512);
				unsigned char* pCRCBytes = (unsigned char*)&crc;
				m_vecTransmitBuffer[4] = *pCRCBytes++;
				m_vecTransmitBuffer[5] = *pCRCBytes;

				m_nValidPageCount++;
			}
			else
			{
				// No more pages to send
				m_vecTransmitBuffer[0] = SRC_RSP_DATA_END;
				m_vecTransmitBuffer[1] = ((unsigned char)(m_nValidPageCount - 1)) & 0xFF;

				printf("\nProcessCommand(GetPageInfo) - No more pages to send, ending successfully\n");

				bUpdateComplete = true;
				m_bUpdateSuccessful = true;
			}
		}
		else
		{
			m_vecTransmitBuffer[0] = SRC_RSP_ERROR;
			bUpdateComplete = true;
		}

		WriteToSerialPort(&m_vecTransmitBuffer[0], m_vecDScommand[DScommandSeq::GetPageInfo]->nCommandTxSize);

		m_bUpdateComplete = bUpdateComplete;
	}
	else if(m_nCommandCode == m_vecDScommand[DScommandSeq::GetPage]->ucCommandCode)
	{
#ifdef DEBUG
		printf("ProcessCommand(GetPage)\n");
#endif

		bool bUpdateComplete(false);

		if(m_bHexImageValid == true)
		{
			m_vecTransmitBuffer[0] = SRC_RSP_OK;
			ExtractHexImageBytes(m_vecTransmitBuffer, 1, (unsigned int)CurrentPageStartAddr(), FlashPageSize);
			m_vecTransmitBuffer[FlashPageSize + 1] = SRC_RSP_OK;

			m_nCurrentPageIndex++; // Increment current page index to prepare for next request
		}
		else
		{
			m_vecTransmitBuffer[0] = SRC_RSP_ERROR;
			bUpdateComplete = true;
		}

		WriteToSerialPort(&m_vecTransmitBuffer[0], FlashPageSize + 2);

		m_bUpdateComplete = bUpdateComplete;
	}
	else
	{
		printf("ProcessCommand(UNKNOWN COMMAND)\n");
	}
}

int CMicrocodeUpdater::CurrentPageStartAddr()
{
	int pageStartAddr = m_nCurrentPageIndex << 9;
	return pageStartAddr;
}

unsigned short CMicrocodeUpdater::GetHexImagePartialCRC(int buf_offset, int numBytes)
{
	unsigned short crc = 0;

	for(int i = 0; i < numBytes; i++)
	{
		crc = UpdateCRC(crc, m_vecHexImage[buf_offset + i]);
	}

	return crc;
}

unsigned short CMicrocodeUpdater::UpdateCRC(unsigned short crc, unsigned char newbyte)
{
	static unsigned short CRCpoly = 0x8408;                 // CRC16-CCITT FCS (X^16+X^12+X^5+1)

	crc = (unsigned short)((unsigned int)crc ^ (unsigned int)newbyte);

	for(int i = 0; i < 8; i++)
	{
		if(((unsigned int)crc & (unsigned int)0x0001) != 0)
		{
			crc = (unsigned short)((unsigned int)crc >> 1);
			crc = (unsigned short)((unsigned int)crc ^ (unsigned int)CRCpoly);
		}
		else
		{
			crc = (unsigned short)((unsigned int)crc >> 1);
		}
	}

	return crc;
}

void CMicrocodeUpdater::ExtractHexImageBytes(std::vector<unsigned char>& outputBuf, int outputBufOffset, unsigned int hexImageStartAddr, int numBytes)
{
	unsigned int numBytesLimit = (unsigned int)numBytes;

	if((hexImageStartAddr + numBytes - 1) > m_uiAppFWendAddr)
	{
		numBytesLimit = m_uiAppFWendAddr - hexImageStartAddr;
		printf("ExtractHexImageBytes() - CHANGING numBytesLimit: %u\n", numBytesLimit);
	}

	outputBuf.clear();
	for(unsigned int i = 0; i < numBytesLimit; i++)
	{
		outputBuf[outputBufOffset + i] = m_vecHexImage[hexImageStartAddr + i];
	}
}

bool CMicrocodeUpdater::OpenPort()
{
	m_fd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY | O_SYNC);
	if(m_fd < 0)
	{
		printf("OpenPort() - open() FAILED\n");
		return false;
	}

	fcntl(m_fd, F_SETFL, 0);	// Normal blocking behavior

	printf("\nPort opened: /dev/ttymxc2\n");

	return true;
}

bool CMicrocodeUpdater::OpenPortForTickler()
{
	m_fd = open("/dev/ttymxc2", O_RDWR | O_NOCTTY | O_NONBLOCK);
	if(m_fd < 0)
	{
		printf("OpenPortForTickler() - open() FAILED\n");
		return false;
	}

	fcntl(m_fd, F_SETFL, FNDELAY);	// Causes read() to return 0 if no data available

	return true;
}

void CMicrocodeUpdater::ClosePort()
{
	if(m_fd > 0)
	{
		close(m_fd);
		m_fd = -1;
	}
}

bool CMicrocodeUpdater::ConfigurePort(int nBaud)
{
	if(m_fd < 0)
	{
		printf("ConfigurePort() - FAILED, m_fd < 0\n");
		return false;
	}

	struct termios tty;
	memset(&tty, 0, sizeof(tty));
	if(tcgetattr(m_fd, &tty) != 0)
	{
		printf("ConfigurePort() - tcgetattr() FAILED\n");
		return false;
	}

	// Input and output speed
	cfsetospeed(&tty, nBaud);
	cfsetispeed(&tty, nBaud);

	// Raw input
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// no parity (8N1)
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag = tty.c_cflag |= (CLOCAL | CREAD | CS8);

	// shut off xon/xoff ctrl (software flow control)
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	// Raw output
	tty.c_oflag &= ~OPOST;

	tty.c_cc[VMIN] = 0;						// read blocks
	tty.c_cc[VTIME] = 50;					// 5 seconds read timeout

	if(tcsetattr(m_fd, TCSAFLUSH, &tty) != 0)
	{
		printf("ConfigurePort() - tcsetattr() FAILED\n");
		return false;
	}

	// NOTE:  VERIFY THIS DOES NOT CAUSE PROBLEMS
	// allow the serial communications to receive SIGIO
	fcntl(m_fd, F_SETOWN, getpid());

	return true;
}

bool CMicrocodeUpdater::ConfigurePortForTickler(int nBaud)
{
	if(m_fd < 0)
	{
		return false;
	}

	struct termios tty;
	memset(&tty, 0, sizeof(tty));
	if(tcgetattr(m_fd, &tty) != 0)
	{
		printf("ConfigurePortForTickler() - tcgetattr() FAILED\n");
		return false;
	}

	// Input and output speed
	cfsetospeed(&tty, nBaud);
	cfsetispeed(&tty, nBaud);

	// Raw input
	tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);

	// no parity (8N1)
	tty.c_cflag &= ~PARENB;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CSIZE;
	tty.c_cflag &= ~CRTSCTS;
	tty.c_cflag = tty.c_cflag |= (CLOCAL | CREAD | CS8);

	// shut off xon/xoff ctrl (software flow control)
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);

	// Raw output
	tty.c_oflag &= ~OPOST;

	tty.c_cc[VMIN] = 5;						// read does not block
	tty.c_cc[VTIME] = 0;

	// These will ensure that your program does not become the 'owner' 
	// of the port subject to sporadic job control and hangup signals, 
	// and also that the serial interface driver will read incoming data bytes. 
	tty.c_cflag |= (CLOCAL | CREAD);

	// no stop bits
	tty.c_cflag &= ~CSTOPB;

	// no flow control
	tty.c_cflag &= ~CRTSCTS;

	if(tcsetattr(m_fd, TCSANOW, &tty) != 0)
	{
		printf("ConfigurePortForTickler() - tcsetattr() FAILED\n");
		return false;
	}

	// NOTE:  VERIFY THIS DOES NOT CAUSE PROBLEMS
	// Allow the serial communications to receive SIGIO
	fcntl(m_fd, F_SETOWN, getpid());

	return true;
}

bool CMicrocodeUpdater::PerformTickle()
{
	bool bOpenPortResult = OpenPortForTickler();
	if(!bOpenPortResult)
	{
		printf("PerformTickle() - OpenPortForTickler() FAILED\n");
		return false;
	}

	bool bConfigureResult = ConfigurePortForTickler(B19200);
	if(!bConfigureResult)
	{
		printf("PerformTickle() - ConfigurePort() FAILED\n");
		return false;
	}

	// Write BOP, Read response
	write(m_fd, "BOP\r\n", 5);
	sleep(1);
	read(m_fd, &m_ReadBuffer[0], 514);
	sleep(1);

	// Write BLS, Read response
	write(m_fd, "BLS\r\n", 5);
	sleep(1);
	read(m_fd, &m_ReadBuffer[0], 514);
	sleep(1);

	ClosePort();

	return true;
}

bool CMicrocodeUpdater::PerformUpdate()
{
	bool bPortOpened = OpenPort();
	if(!bPortOpened)
	{
		printf("\nOpenPort() FAILED\n");
		return false;
	}

	bool bConfigured = ConfigurePort(B115200);
	if(!bConfigured)
	{
		printf("\nConfigurePort() FAILED\n");
		return false;
	}

	bool bReadingResult = StartUpdating(true);
	if(!bReadingResult)
	{
		printf("\nStartUpdating() FAILED\n");
		return false;
	}

	return true;
}

bool CMicrocodeUpdater::StartFirmwareUpdate(string& strFilename)
{
	bool bHexFileOpened = OpenHexFile(strFilename);
	if(!bHexFileOpened)
	{
		printf("\nOpenHexFile() FAILED\n");
		return false;
	}

	bool bTickled = PerformTickle();
	if(!bTickled)
	{
		printf("\nPerformTickle() FAILED\n");
		return false;
	}

	bool bUpdateStarted = PerformUpdate();
	if(!bUpdateStarted)
	{
		printf("\nPerformUpdate() FAILED\n");
		return false;
	}

	return true;
}
