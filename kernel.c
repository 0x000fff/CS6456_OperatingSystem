#define true 1

void printString(char* str);
void readString(char buffer[]);
void readSector(char* buffer, int sector);
void writeSector(char* buffer, int sector);
int myDIV(int dividend, int divisor);
int myMOD(int dividend, int divisor);
void directory();
void deleteFile(char* filename);
void readFile(char* filename, char* outbuf);
void writeFile(char* filename, char* inbuf);
void handleInterrupt21(int AX, int BX, int CX, int DX);
void executeProgram(char* name, int segment);

// interrupt() usage:
// int interrupt (int number, int AX, int BX, int CX, int DX)

void main()
{
    makeInterrupt21();
    interrupt(0x21, 9, "tstprg", 0x2000, 0);
	while(1);
}

void printString(char* str)
{
	int i=0;

	while(*(str+i) != '\0')
	{
		interrupt(0x10, 0xe*256+*(str+i), 0, 0, 0);
		i++;
	}

	// start a new line before printing, use CRLF
	interrupt(0x10, 0xe*256+0xd, 0, 0, 0);
	interrupt(0x10, 0xe*256+0xa, 0, 0, 0);
}

void readString(char buffer[])
{
	int j=0;
	while(true)
	{
		char current_read = interrupt(0x16, 0x0, 0, 0, 0);
		// if it's not backspace (0x8) or enter (0xd), save it
		if(current_read != 0xd && current_read != 0x8)
		{
			buffer[j] = current_read;
			j++;
			// display what entered on stdout (console)
			interrupt(0x10, 0xe*256+current_read, 0, 0, 0);
		}
		// if it is backspace (0x8)
		else if(current_read == 0x8)
		{
			// move one character backwards to drop the last enter char.
			j--;
			// backspace will not erase the deleted character on screen
			// use a blank to overwrite that position and backspace again.
			interrupt(0x10, 0xe*256+0x8, 0, 0, 0);
			interrupt(0x10, 0xe*256+' ', 0, 0, 0);
			interrupt(0x10, 0xe*256+0x8, 0, 0, 0);
		}
		else {
			// add CR to buffer end
			buffer[j] = 0xd;
			// add LF to buffer end
			buffer[j+1] = 0xa;
			buffer[j+2] = 0x0;
			break;
		}
	};
}

void readSector(char* buffer, int sector)
{
	int AX, CX, DX;

	AX = 2*256 + 1;
	CX = myDIV(sector, 36)*256 + ( myMOD(sector, 18) + 1 );
	DX = myMOD(myDIV(sector, 18), 2)*256 + 0;
	interrupt(0x13, AX, buffer, CX, DX);
}

void writeSector(char* buffer, int sector)
{
	int AX, CX, DX;

	AX = 3*256 + 1;
	CX = myDIV(sector, 36)*256 + ( myMOD(sector, 18) + 1 );
	DX = myMOD(myDIV(sector, 18), 2)*256 + 0;
	interrupt(0x13, AX, buffer, CX, DX);
}

int myDIV(int dividend, int divisor)
{
	int result =0;

	if(divisor == 0)
	{
		/* can't divide 0, return error */
		return 1;
	}

	/* circle minus */
	while( (dividend -= divisor) >= 0 )
	{
		++result;
	};

	return result;
}

int myMOD(int dividend, int divisor)
{
	int mod_result;
	// use DIV() to get MOD result
	mod_result = dividend - (divisor * myDIV(dividend, divisor));
	return mod_result;
}

void directory()
{
	int p_row, p_col;
	char dirbuf[512];
	// read out the whole sector #2
	readSector(dirbuf, 2);
	// search the beginning of every 32 bytes
	for(p_row=0; p_row<512; p_row+=32)
	{
		if(dirbuf[p_row] != 0)
		{
			// read out the first 6 bytes of each file entry and print
			for(p_col=p_row; p_col<(p_row+6); p_col++)
				interrupt(0x10, 0xe*256 + dirbuf[p_col], 0, 0, 0);

			interrupt(0x10, 0xe*256+0xd, 0, 0, 0);
			interrupt(0x10, 0xe*256+0xa, 0, 0, 0);
		}
	}
}

void deleteFile(char* filename)
{
	char mapbuf[512], dirbuf[512];
	// use static to initialize with all zero
	static char zerobuf[512];
	int i, j;
	// load map and directory into buffer
	readSector(mapbuf, 1);
	readSector(dirbuf, 2);
	for(i=0; i<512; i+=32)
	{
		// search filename in directory
		if(dirbuf[i] == filename[0] && \
		   dirbuf[i+1] == filename[1] && \
		   dirbuf[i+2] == filename[2] && \
		   dirbuf[i+3] == filename[3] && \
		   dirbuf[i+4] == filename[4] && \
		   dirbuf[i+5] == filename[5])
		{
			// delete file entry in directory
			dirbuf[i] = 0;
			// find occupied sector
			for(j=i+6; j<(i+32); j++)
			{
				// erase corresponding sector
				if(dirbuf[j] != 0)
				{
					writeSector(zerobuf, dirbuf[j]);
					// update map in sector #1
					mapbuf[dirbuf[j]] = 0;
				}
			}
		}
	}
	// write back to update disk
	writeSector(mapbuf, 1);
	writeSector(dirbuf, 2);
}

void readFile(char* filename, char* outbuf)
{
	char dirbuf[512];
	int i, j, m=0;
	// load map and directory into buffer
	readSector(dirbuf, 2);
	for(i=0; i<512; i+=32)
	{
		// search filename in directory
		if(dirbuf[i] == filename[0] && \
		   dirbuf[i+1] == filename[1] && \
		   dirbuf[i+2] == filename[2] && \
		   dirbuf[i+3] == filename[3] && \
		   dirbuf[i+4] == filename[4] && \
		   dirbuf[i+5] == filename[5])
		{
			// find occupied sector
			for(j=i+6; j<(i+32); j++)
			{
				if(dirbuf[j] != 0)
				{
					readSector((outbuf+m*512), dirbuf[j]);
					m++;
				}
			}
		}
	}
}

void writeFile(char* filename, char* inbuf)
{
	char mapbuf[512], dirbuf[512];
	int i, j, n;
	// load map and directory into buffer
	readSector(mapbuf, 1);
	readSector(dirbuf, 2);
	// search for available sectors
	for(n=0; n<512; n++)
	{
		if(mapbuf[n] == 0)
		{
			mapbuf[n] = 0xFF;
			break;
		}
	}
	// search for available file entry in directory
	for(i=0; i<512; i+=32)
	{
		if(dirbuf[i] == 0)
			break;
	}
	// write filename into directory
	for(j=i; j<(i+6); j++)
	{
		dirbuf[j] = filename[j-i];
	}
	// write the occupied sector into file entry
	dirbuf[j+1] = n;

	writeSector(mapbuf, 1);
	writeSector(dirbuf, 2);
	// write data into corresponding sector
	writeSector(inbuf, dirbuf[j+1]);
}

void executeProgram(char* name, int segment)
{
    char filebuf[512];
    readFile(name, filebuf);
}

void handleInterrupt21(int AX, int BX, int CX, int DX)
{
	switch(AX)
	{
		case 0:
			printString(BX);
			break;
		case 1:
			readString(BX);
			break;
		case 2:
			readSector(BX, CX);
			break;
		case 3:
			printString("List directory:");
			directory();
			break;
		case 4:
			deleteFile(BX);
			break;
		case 6:
			// BX is filename, CX is buffer pointer
			readFile(BX, CX);
			break;
		case 7:
			writeSector(BX, CX);
			break;
		case 8:
			writeFile(BX, CX);
			break;
		case 9:
			executeProgram(BX, CX);
			break;
		default:
			printString("Syscall not supported");
			break;
	}
}
