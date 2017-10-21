/*

iBooks Pro C
Program main source file

(c)2013 - 2017 Xhorizon, Some rights reserved.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include <display_syscalls.h>
#include <keyboard_syscalls.h>
#include <stdio.h>
#include <stdlib.h>

#include "image.h"
#include "fxchslib.h"
#include "browser.h"
#include "keybios.h"

int Bfile_OpenFile_OS	(const unsigned short*filename, int mode );
int Bfile_CloseFile_OS	(int HANDLE );
int Bfile_ReadFile_OS	(int HANDLE, void *buf, int size, int readpos );
int Bfile_GetFileSize_OS( int handle, int pos );

int Bfile_WriteFile_OS( int HANDLE, const void *buf, int size );
int Bfile_CreateEntry_OS( const unsigned short*filename, int mode, int*size );
int Bfile_SeekFile_OS( int handle, int pos );

FONTFILE* font16;
int totbytes=0,page=0,cached=0;    // ��ȡ�ֽ�ָ�롢��ǰҳ�롢�ѻ���ҳ��
unsigned int bytes[9999];    // ҳ�滺��
unsigned int bookmark[3];    // ��ǩ

int divide_page(const char* filename,const int pages,int save)
{
    /*
	    ��ҳ����
        ����˵����
	        filename: ��ǰ���ļ����ļ���
            pages: ��Ҫ�ֵ�ҳ��
            save: �Ƿ���Ҫ���ص�һҳ (��ʼ��ʱʹ��)
		����ֵ��
		    0: �Ѷ����ļ�ĩβ
			1: δ�����ļ�ĩβ
    */

    int cx,cy;
    int i=0,j;
    const int pp=16;
	int is_chs=0;  // �����ַ���ʶ
	int tmp=cached,total=0;
	
	int decades_passed = 1;
	
	int handle;
	char* buf=(char*)malloc(460);
	FONTCHARACTER fname[64];
	
    char_to_font(filename,fname);
	handle=Bfile_OpenFile_OS(fname,0);
	tmp=cached;
	totbytes=bytes[cached];  // ��������
	
	// ��������ҳ������ 9999 ҳ������Ϊ 9999 ҳ
	if ((total=tmp+pages)>9999) total=9999;
	// �� 9999 ҳʱ��������һҳ������
	if (cached+1>9999) {Bfile_CloseFile_OS(handle);return 0;};
	
	// ����Ļ����ʾ��ҳ�Ľ���
	ProgressBar(0, total - tmp);
	
	for (j=tmp;j<total;++j)    //  �ӵ�ǰ�ѻ���ҳ��ֵ������ҳ�棬ʹ��ģ�������ʾ���򷽷�
	{
	    // ���Զ�ȡһ�������Ա���ҳ
	    memset(buf,0,461);
	    Bfile_ReadFile_OS(handle,buf,400,totbytes);
		// ��������ļ�ĩβ������
		if (!buf[0])
		{
			ProgressBar(total - tmp, total - tmp);
			Bfile_CloseFile_OS(handle);
			MsgBoxPop();
			return 0;
		}
		
		// ���λ������Ϊ��ʼ״̬
        cx=0;
		cy=24;
		
        for (i=0;;)
        {
            is_chs=buf[i] & 0x80;  // �жϸ�λ�ֽڣ�ȷ���Ƿ�Ϊ����
            if ((cx+pp)>368)  // ��䳬����Ļ�ұ�Ե
                goto cn;
            if (is_chs) i+=2;  // ���ģ���2�ֽ�
            else
            {
		        if (buf[i] == '\r' || buf[i] == '\n')  // �������س���ֱ�ӽ�����һ��
			    {
			        i++;
					if (buf[i] == '\r' || buf[i] == '\n')  // �������س���ֱ�ӽ�����һ��
						i++;
				    goto cn;
			    }
			    i++;
            }
        
		    // ���ַ�����뵱ǰ�У������ַ�ƫ��
		    if (is_chs)
                cx+=25;
		    else
		        cx+=18;
        
		    // ��䳬����Ļ�ұ�Ե
            if (cx>368)
            {
            cn:
                cx=0;
                cy+=24;  // ���������ַ�ƫ�ƣ�������һ��
			    if (cy>190)  // ��䳬����Ļ�±�Ե������
			        break;
            }
        }
	    bytes[j+1]=i+totbytes;  // �����һ���ַ����ļ��е�λ�ô�����һҳ�Ļ��棬�Ա���ȡ
		totbytes+=i;  // ��ȡ�ֽ�ָ������
	    ++cached;  // �ѻ���ҳ�����ӣ���ʾ��ҳ�ɹ�
		
		// ÿ����1/10�����壬���ӽ�����ʾ
		if (j - tmp == (total - tmp) / 10 * decades_passed)
		{
			ProgressBar(j - tmp, total - tmp);
			decades_passed++;
		}
	}
	if (save) page=0;    // ���ص�һҳ  
	
	ProgressBar(total - tmp, total - tmp);
	Bfile_CloseFile_OS(handle);
	MsgBoxPop();
	return 1;
}

void Save_Config(const char* fn,int n)
{
    /*
	    �洢��ҳ����ǩ����
		����˵����
		    fn: ��ǰ���ļ����ļ���
		    n: �ѻ���ҳ��
	*/

    char tmp[64];
	FONTCHARACTER fname[64];
	int handle,size=0;

    memset(tmp,0,sizeof(tmp));
	strncpy(tmp,fn,strlen(fn)-strlen(strrchr(fn,'.')));    // ȡ�ļ�������
	strcat(tmp,".cfg");
	
	char_to_font(tmp,fname);
	Bfile_CreateEntry_OS(fname,1,&size);    // ���� .cfg �ļ�
	handle=Bfile_OpenFile_OS(fname,2);
	if (handle<=0) return;
	
	Bfile_WriteFile_OS(handle,&n,4);    // ǰ 4 �ֽڣ�д���ѻ���ҳ��
	
	Bfile_SeekFile_OS(handle,4);
	Bfile_WriteFile_OS(handle,bookmark,16);    // 4*4 �ֽڣ�д����ǩָ���ҳ��
	
	Bfile_SeekFile_OS(handle,20);
	Bfile_WriteFile_OS(handle,bytes,n*4);    // 4*n �ֽڣ�д��ҳ�滺��
	Bfile_CloseFile_OS(handle);
}

void Read_Config(const char* fn,int* n)
{
    /*
	    ��ȡ��ҳ����ǩ����
		����˵����
		    fn: ��ǰ���ļ����ļ���
		    n: �����ѻ���ҳ���Ļ�����
	*/

    char tmp[64];
	FONTCHARACTER fname[64];
	int handle,_n=*n;

    memset(tmp,0,sizeof(tmp));
	strncpy(tmp,fn,strlen(fn)-strlen(strrchr(fn,'.')));    // ȡ�ļ�������
	strcat(tmp,".cfg");
	
	char_to_font(tmp,fname);
	handle=Bfile_OpenFile_OS(fname,0);
	*n=_n;
	if (handle<=0) return;
	Bfile_ReadFile_OS(handle,&_n,4,0);*n=_n-1;    // �����ѻ���ҳ��
	Bfile_ReadFile_OS(handle,bookmark,16,4);    // ������ǩҳ��
	Bfile_ReadFile_OS(handle,bytes,_n*4,20);    //����ҳ�滺��
	
	Bfile_CloseFile_OS(handle);
}

void Save_Bookmark(const char* fn,unsigned int pages,int n)
{
    /*
	    �洢��ǩ��������ƣ�
		����˵����
		    fn: ��ǰ���ļ����ļ���
		    pages: ��ǰ����ҳ��
		    n: �ѻ���ҳ����
	*/

    int handle,size,i=0;
	int sel=0,flag;
	char tip[64],tmp[64];
	
	MsgBoxPush(5);
	
	beg:
	flag=0;
	
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0,192,384,216);
	draw_pic(0,192,124,22,0,Menu_PutB);
	
	ClearArea(35,49,349,166);
	print_chs_2(38,47,0,"�洢��ǩ");
	
	close_font(font16);
	for (i=0;i<4;++i)
	{
	    memset(tmp,0,sizeof(tmp));memset(tip,0,sizeof(tip));
	    strcat(tip,"[");
	    itoa(i+1,tmp,10);strcat(tip,tmp);
		strcat(tip,"]");
        if (bookmark[i])
		{
		    strcat(tip,"     Page ");
		    memset(tmp,0,sizeof(tmp));
		    itoa(bookmark[i],tmp,10);strcat(tip,tmp);
		}
		else strcat(tip,"     Empty");
		locate_OS(3,3+i);
		Print_OS(tip,0,0);
	}

	Bdisp_AreaReverseVRAM(35,72+sel*24,349,94+sel*24);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_UP:
			    if (sel>0) --sel;
				goto beg;break;
				
			case KEY_CTRL_DOWN:
			    if (sel<3) ++sel;
				goto beg;break;
			
			case KEY_CTRL_F1:
	        case KEY_CTRL_EXE:
				bookmark[sel]=pages+1;    // �洢��ǩ
			    goto prg;break;
			
			case KEY_CTRL_F2:
			case KEY_CTRL_DEL:
			    bookmark[sel]=0;flag=1;goto prg;break;    // ɾ����ǩ
			
			case KEY_CTRL_EXIT:
				MsgBoxPop();
			    return;
		}
	}
	
	prg:
	Save_Config(fn,n);
	
	if (flag) goto beg;
	MsgBoxPop();
}

void Read_Bookmark(const char* fn,int* pages,int* n)
{
    /*
	    ��ȡ��ǩ��������ƣ�
		����˵����
		    fn: ��ǰ���ļ����ļ���
		    pages: ���ܵ�ǰ����ҳ��Ļ�����
		    n: �����ѻ���ҳ�����Ļ�����
	*/

    int handle,_n=0,_page=*pages;
	int i,sel=0;
	FONTCHARACTER fname[64];
	char tip[64],tmp[64];
	
	MsgBoxPush(5);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0,192,384,216);
	draw_pic(0,192,61,22,0,Menu_ReadB);
	
	ClearArea(35,49,349,166);
	print_chs_2(38,47,0,"��ȡ��ǩ");
	
	close_font(font16);
	for (i=0;i<4;++i)
	{
	    memset(tmp,0,sizeof(tmp));memset(tip,0,sizeof(tip));
	    strcat(tip,"[");
	    itoa(i+1,tmp,10);strcat(tip,tmp);
		strcat(tip,"]");
        if (bookmark[i])
		{
		    strcat(tip,"     Page ");
		    memset(tmp,0,sizeof(tmp));
		    itoa(bookmark[i],tmp,10);strcat(tip,tmp);
		}
		else strcat(tip,"     Empty");
		locate_OS(3,3+i);
		Print_OS(tip,0,0);
	}

	Bdisp_AreaReverseVRAM(35,72+sel*24,349,94+sel*24);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_UP:
			    if (sel>0) --sel;
				goto beg;break;
				
			case KEY_CTRL_DOWN:
			    if (sel<3) ++sel;
				goto beg;break;
				
	        case KEY_CTRL_EXE:
			    if (bookmark[sel])
				{
			        *pages=bookmark[sel]-1;    // ��ȡ��ǩ
					MsgBoxPop();
			        return;
				}
				else goto beg;break;
			
			case KEY_CTRL_F1:
			case KEY_CTRL_EXIT:
			    *pages=_page;
				MsgBoxPop();
			    return;break;
		}
	}
}

void Confirm_AllDivide(const char* fn)
{
    /*
	    ����ȫ����ҳ��������ƣ�
		����˵����
		    fn: ��ǰ���ļ����ļ���
	*/

	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(35,45,349,166);
	print_chs_2(38,47,0,"����ȫ����ҳ������Ҫ");
	print_chs_2(38,71,0,"�ܳ�ʱ�䡣");
	print_chs_2(38,95,0,"ȷ��������");
	
	locate_OS(6,5);
	Print_OS("[F1]:",0,0);print_chs_2(181,119,0,"��");
	locate_OS(6,6);
	Print_OS("[F6]:",0,0);print_chs_2(181,143,0,"��");
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_F1:
			    divide_page(fn,9999-cached,0);    // ���Էֵ� 9999 ҳ
				return;break;
			case KEY_CTRL_F6:
				return;break;
	    }
	}
}

void Disp_About()
{
    /*
	    ��ʾ������Ϣ��������ƣ�
	*/

	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	MsgBoxPush(5);
	
	ClearArea(35,45,349,166);
	locate_OS(3,2);
	Print_OS("iBooks Pro C",0,0);
	print_chs_2(38, 71, 0, "�汾 ");
	locate_OS(7, 3);
	Print_OS("1.50", 0, 0);
	print_chs_2(38, 95, 0, "��������ˮ��Ұ������");
	print_chs_2(38, 119, 0, "��������");
	locate_OS(10, 5);
	Print_OS("GNU GPL v3",0,0);
	print_chs_2(38, 143, 0, "Э�鿪��Դ���롣");
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_EXIT:
				MsgBoxPop();
				return;break;
	    }
	}
}

void Page_Jump(const char* fn)
{
    /*
	    ��ҳ��������ƣ�
		����˵����
		    fn: ��ǰ���ļ����ļ���
	*/

	FONTCHARACTER fname[64];
	char tip[64],tmp[64];
	char keybuff[32];    //  ����ҳ����ַ�������
	int inspos=0,target;
	
	memset(keybuff, 0, sizeof(keybuff));
	MsgBoxPush(5);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0,192,384,216);
	draw_pic(0,192,61,22,0,Menu_Sub_Jump);
	draw_pic(63,192,61,22,0,Menu_Jump);
	
	ClearArea(35,45,349,166);
	
	print_chs_2(38,47,0,"��ǰҳ/�ѻ���ҳ����");
	
	locate_OS(3,3);
	memset(tmp,0,sizeof(tmp));memset(tip,0,sizeof(tip));
	itoa(page+1,tmp,10);strcat(tip,tmp);
    strcat(tip,"/");memset(tmp,0,sizeof(tmp));
	itoa(cached,tmp,10);strcat(tip,tmp);
	Print_OS(tip,0,0);
	
	print_chs_2(38,119,0,"����Ŀ��ҳ�룺");
	locate_OS(3,6);
	Print_OS("[    ]",0,0);
	locate_OS(4,6);
	Print_OS(keybuff,0,0);
	
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
			case KEY_CTRL_EXIT:
				MsgBoxPop();
			    return;break;
			
			case KEY_CTRL_F2:
			    Confirm_AllDivide(fn);    // ����ȫ����ҳ
				goto beg;break;
			
			case KEY_CTRL_DEL:
			    if (inspos>0)
				{
					keybuff[--inspos]=0;    // ���һ���ֽ���Ϊ NULL����ʶ��ɾ��
					goto beg;
				}
			    break;
			
			case KEY_CTRL_F1:
			case KEY_CTRL_EXE:
			    if (inspos==0) break;
				target=atoi(keybuff);
				if (target>cached)    // ���Ŀ��λ�ó����ѻ���ҳ���������Է�ҳ
				{
				    if (!divide_page(fn,target-cached,1)) page=cached-1;    // ����ҳ�Ѵ��ļ�ĩβ���򽫵�ǰҳ�����������һҳ
					else page=target-1;    // ����������ǰҳ��Ϊ�����Ŀ��λ��
			    }
				else
				    page=target-1;    // ������ǰҳ��Ϊ�����Ŀ��λ��
				MsgBoxPop();
				return;break;
			
			default:
			    if (key>=0x30&&key<=0x39)    // �������� 0~9
				{
				    if (inspos<=3)
					{
					    if (key==0x30&&inspos==0) break;    // ���Ե�һλ���� 0 ʱ����
						keybuff[inspos++]=key;    // ���һ���ֽ��������
						goto beg;
					}
				}
		}
	}
}

int Subdir_Open(const char* fn)
{
    /*
	    ��Ŀ¼�ļ������루������ƣ�
		����˵����
		    fn: ���մӼ��������ļ����Ļ�����
	*/

	FONTCHARACTER fname[64];
	char keybuff[32];    //  �����ļ������ַ�������
	int inspos = 0;
	
	memset(keybuff, 0, sizeof(keybuff));
	MsgBoxPush(4);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(0, 192, 384, 216);
	draw_pic(0, 192, 61, 22, 0, Menu_Sub_Jump);
	
	ClearArea(35, 45, 349, 141);
	print_chs_2(38, 71, 0, "�������򿪵��ļ�����");
		
	locate_OS(3, 4);
	Print_OS("[        ].txt", 0, 0);
	locate_OS(4, 4);
	Print_OS(keybuff, 0, 0);
	
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
			case KEY_CTRL_EXIT:
				MsgBoxPop();
			    return 0; break;
						
			case KEY_CTRL_DEL:
			    if (inspos > 0)
				{
					keybuff[--inspos] = 0;    // ���һ���ֽ���Ϊ NULL����ʶ��ɾ��
					goto beg;
				}
			    break;
			
			case KEY_CTRL_F1:
			case KEY_CTRL_EXE:
			    strcpy(fn, keybuff);
				//MsgBoxPop();
				return 1; break;
			
			default:
			    if (key >= 0x30 && key <= 0x39 || key >= 0x41 && key <= 0x5A || key >= 0x61 && key <= 0x7A)    // �������� 0~9 ��Ӣ����ĸ a-z��A-Z
				{
				    if (inspos <= 8)
					{
						keybuff[inspos++] = key;    // ���һ���ֽ��������
						goto beg;
					}
				}
		}
	}
}

void Disp_FileNotFound()
{
    /*
	    ��ʾ���ļ�δ�ҵ�����Ϣ��������ƣ�
	*/

	//MsgBoxPush(4);
	
	beg:
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	ClearArea(35, 45, 349, 141);
	print_chs_2(94, 71, 0,"�ļ�δ�ҵ�");
	
	print_chs_2(94, 119, 0, "��");
	locate_OS(8, 5);
	Print_OS(":[EXIT]", 0, 0);
	close_font(font16);
	
	while (1)
	{
	    int key;
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_EXIT:
				//MsgBoxPop();
				return; break;
	    }
	}
}

void aa(int* pos,int* firstn,const int n)
{
    /*
	    �б������ƴ�����
		����˵����
		    pos: ��ǰ�б�������λ�� (0 ��ʼ)
			firstn���б����Ƶ����� (0 ��ʼ)
			n: �б����Ŀ����
	*/

    if ((*pos+*firstn)>0)    // ����б����Ѿ����ƣ����ƹ��
				{
				    --(*pos);    // ���ƹ��
					if (*pos<0)    //  �������Ƶ��˶���
					{
					    *pos=0;
						--(*firstn);    // �б���������
					}
				}
	else    // ����б����ڵ�һ�ѭ������
	            {
				    if (n>6)    //  ����б���������һ��
					{
                        *pos=5;
					    *firstn=n-6;    //  ���������б�����һ�� (�ײ�)
					}
					else
					{
					    *pos=n-1;
						*firstn=0;    //  ���������б�����һ��
					}
                }				
}

void bb(int* pos,int* firstn,const int n)
{
    /*
	    �б��������ƴ�����
		����˵����
		    pos: ��ǰ�б�������λ�� (0 ��ʼ)
			firstn���б����Ƶ����� (0 ��ʼ)
			n: �б����Ŀ����
	*/
	
    if ((*pos+*firstn)<n-1)    //  ����б���δ��ĩβ�����ƹ��
				{
                    ++(*pos);    //  ���ƹ��
					if (*pos>5)    //  �������Ƴ�һ���ķ�Χ
					{
					    *pos=5;    //  �������λ��
						++(*firstn);    //  �б���������
					}
				}
				else    //  ����Ѿ��Ƶ�ĩβ��ѭ������
				{
				    *pos=0;
					*firstn=0;    //  ���ص���һ��
				}
}

void iRead_main(const char* filename)
{
    /*
	    �Ķ�����������
		����˵����
		    filename: �򿪵��ļ��� (���ļ�������õ�)
	*/

    int key,handle;
	char* buf=(char*)malloc(461);
	FONTCHARACTER fname[64];
	
	char tip[64], tmp[64];
	
	page=0;cached=0;
	
	memset(bytes,0,sizeof(bytes));
	memset(bookmark,0,sizeof(bookmark));bookmark[3]=0;
	
	Read_Config(filename,&cached);    //  ��ȡ��ǩ����ҳ����
	
	//  ����ֵ�ҳ������ 500 ��������������ҳ��
	if (cached==0) divide_page(filename,500-cached,1);
	else
	    if (cached%500!=0) divide_page(filename,500-cached%500,1);   // ���� 500 ��������
	totbytes=0;
	
	/*  ����״̬����ʾ����
		0x0001����ʾ����
		0x0100����ʾ����
	*/
	DefineStatusAreaFlags(3, 0x01 | 0x02 | 0x100, 0, 0);
	
	beg:
	font16=open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	Bdisp_AllClr_VRAM();
    draw_pic(0,192,124,22,0,Menu_Read);
	draw_pic(126,192,61,22,0,Menu_Sub_Jump);
	
	//  ������һҳʱ�����ѻ���ҳ�淶Χ
	if (cached<=page)
	{
	    //  ����ֵ�ҳ������ 500 ��������������ҳ��
	    if (!divide_page(filename,1,0)) page=cached-1;
	    else if (cached%500!=0) divide_page(filename,500-cached%500,0);
		
		close_font(font16);
		goto beg;
	}
	totbytes=bytes[page];    //  ������ȡ�ֽ�ָ��λ��
	
	char_to_font(filename,fname);
	handle=Bfile_OpenFile_OS(fname,0);    //  ���ļ�
	
	Bfile_ReadFile_OS(handle,buf,400,totbytes);
	Bfile_CloseFile_OS(handle);
	
	print_chs_page(0,24,totbytes,(unsigned char*)buf);    //  ����һҳ
    close_font(font16);
	
	//  ׼����ʾ�������
	char fn_ptr[64];
	memset(fn_ptr, 0, sizeof(fn_ptr));
	GetDisplayFileName(filename, fn_ptr);
	
	memset(tip, 0, sizeof(tip));
	memset(tmp, 0, sizeof(tmp));
	strcat(tip, fn_ptr); strcat(tip, " ");
	itoa(page + 1, tmp, 10); strcat(tip, tmp);
    strcat(tip, "/"); memset(tmp, 0, sizeof(tmp));
	itoa(cached, tmp, 10);strcat(tip, tmp);
	
	//  ״̬����ʾ�ļ���������
	DefineStatusMessage(tip, 0, 0, 0);
	
	while (1)
	{
	    GetKey(&key);
	    switch (key)
		{
		    case KEY_CTRL_UP:    //  ������һҳ
			    if (page>0)
				{
			        --page;
				    goto beg;
				}
				break;
				
		    case KEY_CTRL_DOWN:    //  ������һҳ
			    ++page;
			    goto beg;
				break;
				
			case KEY_CTRL_EXIT:    //  �뿪�������ļ������
			    Save_Config(filename,cached+1);
				DefineStatusAreaFlags(3, 0x01 | 0x02 | 0x100, 0, 0);
			    return;break;
				
			case KEY_CTRL_F2:    //  �򿪴洢��ǩ�Ի���
			    Save_Bookmark(filename,page,cached+1);
				goto beg;break;
				
			case KEY_CTRL_F1:    //  �򿪶�ȡ��ǩ�Ի���
			    Read_Bookmark(filename,&page,&cached);
				goto beg;break;
				
			case KEY_CTRL_F3:    //  ����ҳ�Ի���
			    Page_Jump(filename);
				goto beg;break;
		}
 	}
}

int cmp(const void *a, const void *b)
{
	/*
	    �����ļ�������ıȽϺ���
	*/
	char aa[100], bb[100];
	char *oo;
	strcpy(aa, ((const f_name *)a) -> name); strcpy(bb, ((const f_name *)b) -> name);
	
	if (oo = strchr(aa, '[')) *oo = ' ';
	if (oo = strchr(bb, '[')) *oo = ' ';
	
    return strcmp(aa, bb);
}

void browse_main()
{
    /*
	    �ļ������������
	*/

    char ncat[64], workdir[64] = "\\\\fls0";    //  ��ǰĿ¼
    f_name *a=get_file_list("\\\\fls0\\*.*");    //  �洢�ļ��б�Ķ�ά����
	int pos=0,firstn=0;    //  �б���λ�á��б����Ƶ�����
	unsigned int key;
	char subdir_fn[32];    //  ��������Ŀ¼�ļ�������Ļ�����
	FONTCHARACTER fname[64];
	int handle = 0;
	
	DefineStatusAreaFlags(3, 0x01 | 0x02 | 0x100, 0, 0);
	
	beg:
	if (a) qsort(a, getn(a), sizeof(char *), cmp);
	
	font16 = open_font("\\\\fls0\\24PX.hzk");
	select_font(font16);
	
	draw_browser(workdir,firstn,pos,a);    //  �������������
	
	close_font(font16);
	
	//  ��ʾ��ǰ����Ŀ¼��״̬��
	if (strcmp(workdir, "\\\\fls0") == 0)
		DefineStatusMessage("", 0, 0, 0);
	else
	{
		memset(ncat, 0, sizeof(ncat));
		GetDisplayDirName(workdir, ncat);
		DefineStatusMessage(ncat, 0, 0, 0);
	}
	
	while (1)
	{
	    GetKey(&key);
		switch (key)
		{
		    case KEY_CTRL_UP:    //  �������
			    if (a)
				{
				    aa(&pos,&firstn,getn(a));
					goto beg;
				}
				break;
				
			case KEY_CTRL_DOWN:    //  �������
			    if (a)
				{
				    bb(&pos,&firstn,getn(a));
					goto beg;
				}
				break;
			
			case KEY_CTRL_F6:    //  ��ʾ������Ϣ
			    Disp_About();
				goto beg;
				break;
			
			case KEY_CTRL_F1:    //  �򿪹��λ�õ��ļ�
			case KEY_CTRL_EXE:
			    if (a)    //  ����ļ��б�Ϊ��
			    {
			        if (strchr(a[pos+firstn].name,'['))    //  ����򿪵����ļ���
				    {
				        memset(ncat,0,sizeof(ncat));
					    //strcat(ncat,"\\\\fls0\\");
						strcat(ncat, workdir); strcat(ncat, "\\");
				        strcat(ncat, ++a[pos+firstn].name);
					    memset(workdir, 0, sizeof(workdir));
					    strcpy(workdir, ncat);
					    strcat(ncat, "\\*.*");    //  �������ļ�������
					    a=get_file_list(ncat);    //  ������ļ���
						pos=0; firstn=0;    //  �б��ʼ��
					    goto beg;
				    }
				    else    //  ����򿪵����ı��ļ�
				    {	
						memset(ncat,0,sizeof(ncat));
				        strcpy(ncat,workdir);
						strcat(ncat,"\\");
				        strcat(ncat,a[pos+firstn].name);    //  �������ļ�����
						
						iRead_main(ncat);    //  �����Ķ���
						goto beg;
				    }
				}
				break;
				
			case KEY_CTRL_F2:	//  ����������ļ������ļ�
				memset(subdir_fn, 0, sizeof(subdir_fn));
				if (Subdir_Open(subdir_fn))
				{
					memset(ncat, 0, sizeof(ncat));
				    strcpy(ncat, workdir);
					strcat(ncat, "\\");
				    strcat(ncat, subdir_fn);    //  ������������ļ�����
					strcat(ncat, ".txt");
						
					char_to_font(ncat, fname);
					handle = Bfile_OpenFile_OS(fname,0);
					if (handle <= 0)    //  ����ļ�δ�ҵ�
					{
						Disp_FileNotFound();
						MsgBoxPop();
						goto beg; break;
					}
						
					MsgBoxPop();
					Bfile_CloseFile_OS(handle);
					
					//  ���»������������
					font16 = open_font("\\\\fls0\\24PX.hzk");
					select_font(font16);
	
					draw_browser(workdir, firstn, pos, a);
					close_font(font16);
					
					//  �����Ķ���
					iRead_main(ncat);
				}
				
				goto beg; break;
			
			case KEY_CTRL_EXIT:    //  ���ļ��з��ظ�Ŀ¼
			    if (strcmp(workdir,"\\\\fls0")!=0)    //  �����ǰ���ļ�����
			    {
			        memset(ncat,0,sizeof(ncat));
			        strncpy(ncat,workdir,strlen(workdir)-strlen(strrchr(workdir,'\\')));
				    memset(workdir,0,sizeof(workdir));
				    strcpy(workdir,ncat);
				    strcat(ncat,"\\*.*");    //  ��������һ��Ŀ¼������
			        a=get_file_list(ncat);    //  ������ļ���
					pos=0;firstn=0;    //  ��ʼ���б�
				    goto beg;
				
				}
				break;
			
		}
	}
}

void GetDisplayDirName(char *src, char *des)
{
	/*
	    �ӹ���Ŀ¼�л�ȡ��Ҫ��ʾ���ļ��в����Ϣ
		����˵����
			src: ������Ĺ���Ŀ¼
			des: ��������Ϣ��ŵĻ�����
	*/
	
	char ncat[64], idm[32];
	char *s;
	
	memset(ncat, 0, sizeof(ncat));
	
	for (s = src + 7; (int)s != 1; s = strchr(s, '\\') + 1)
	{
		strcat(ncat, "\\\\");
		memset(idm, 0, sizeof(idm));
		strncpy(idm, s, strlen(s) - (strchr(s, '\\') ? strlen(strchr(s, '\\')) : 0));
		strcat(ncat, idm);
	}
	
	strcpy(des, ncat);
}

void GetDisplayFileName(char *src, char *des)
{
	/*
	    �ӹ���Ŀ¼�л�ȡ��Ҫ��ʾ���ļ�����
		����˵����
			src: ������Ĺ���Ŀ¼
			des: �����Ϣ��ŵĻ�����
	*/
	
	strcpy(des, strrchr(src, '\\') + 1);
}

int check_consistency()
{
    /*
	    ����ļ�һ���ԣ����ļ���ȱʧ����ʾ������Ϣ
	*/

    FONTCHARACTER fname[64];
	int handle,i;
	char_to_font("\\\\fls0\\24PX.hzk",fname);
    handle=Bfile_OpenFile_OS(fname,0);
	if (handle<0) return 0;
	Bfile_CloseFile_OS(handle);
	
	return 1;
}

void main(void) {

    /*
	    ����������
	*/

    int key;
    if (!check_consistency())    //  ����ļ���ȱʧ����ʾ������Ϣ���˳�
	{
	    locate_OS(1,1);
		Print_OS("Can't find font file.",0,0);
		while (1) GetKey(&key);
	}
	
	browse_main();    //  ��������ļ������
	return;
}