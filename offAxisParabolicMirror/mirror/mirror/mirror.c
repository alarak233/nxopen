/*****************************************************************************
**
** mirror.c
**
** Description:
**     Contains Unigraphics entry points for the application.
**
*****************************************************************************/

/* Include files */
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <uf.h>
#include <uf_ui_types.h>
#include <uf_ui.h>
#include <uf_part.h>
#include <direct.h> 
#include <uf_modl.h>

static void ECHO(char *format, ...)
{
    char msg[UF_UI_MAX_STRING_BUFSIZE];
    va_list args;
    va_start(args, format);
    vsnprintf_s(msg, sizeof(msg), _TRUNCATE, format, args);
    va_end(args);
    UF_UI_open_listing_window();
    UF_UI_write_listing_window(msg);
    UF_print_syslog(msg, FALSE);
}

#define UF_CALL(X) (report_error( __FILE__, __LINE__, #X, (X)))

static int report_error( char *file, int line, char *call, int irc)
{
    if (irc)
    {
        char err[133];

        UF_get_fail_message(irc, err);
        ECHO("*** ERROR code %d at line %d in %s:\n",
            irc, line, file);
        ECHO("+++ %s\n", err);
        ECHO("%s;\n", call);
    }

    return(irc);
}


/*****************************************************************************
**  Activation Methods
*****************************************************************************/
/*  Explicit Activation
**      This entry point is used to activate the application explicitly, as in
**      "File->Execute UG/Open->User Function..." */
extern DllExport void ufusr( char *parm, int *returnCode, int rlen )
{
    /* Initialize the API environment */
    if( UF_CALL(UF_initialize()) ) 
    {
        /* Failed to initialize */
        return;
    }
    
    /* TODO: Add your application code here */
	/*全局变量*/

//	int response;
	int units = UF_PART_METRIC;//公制单位
	int i = 0;

	/*1.选择反射镜形状（圆形，方形）*/
	
	int isCircle = 5;	//uc1603返回值5-18
	char shape[2][38] = {"圆形","方形"};
	isCircle = uc1603("请选择抛物镜形状",0,shape,2);
	if (isCircle == 1 || isCircle == 2)
		UF_CALL(UF_terminate());

//	测试对话框
/*	
	char charIsCircle[30];
	UF_UI_open_listing_window();
	if (isCircle == 5)
	{
		UF_UI_write_listing_window("圆形\n");
	}
	else if (isCircle == 6)
	{
		UF_UI_write_listing_window("方形\n");
	}
	sprintf(charIsCircle, "%d", isCircle);
	UF_UI_write_listing_window(charIsCircle);
*/
	/*2.输入所需的参数：
		1）.离轴距离d
		2）.抛物线关键参数y=1/2p*x^2
		3）.反射镜高度h或反射镜中心高度ho
		4）.反射镜底面大小（圆形r，方形a，b，其他类型自改）*/

	double d,p,h,r,a,b;
	char circleSize[4][16] = { "离轴距离","焦距(p)y=1/px^2","总高度","底部半径" };
	char squareSize[5][16] = { "离轴距离","焦距(p)y=1/px^2","总高度","长（沿轴向）","宽（垂直轴向）" };
	double data[5] = { 127,254,25.4,12.7,12.7 };
	int *unused = 0;
	if (isCircle == 5)
	{
		uc1609("请输入尺寸", circleSize, 4, data, unused);
		d = data[0];
		p = data[1] / 2;
		h = data[2];
		r = data[3];
	}
	else
	{
		uc1609("请输入尺寸", squareSize, 5, data, unused);
		d = data[0];
		p = data[1] / 2;
		h = data[2];
		a = data[3];
		b = data[4];
	}

//	测试对话框
/*
	char charData[5][16];
	int i = 1;
	UF_UI_open_listing_window();
	for ( i = 0; i < 5; i++)
	{
		sprintf(charData[i], "%f", data[i]);
		UF_UI_write_listing_window(charData[i]);
		UF_UI_write_listing_window("\n");
	}
*/

	/*3.判断工装类型（吸盘式，螺丝固定）*/
	
	int isSucker = 1;
	if (d<68.2)
	{
		isSucker = 0;
	}
	

	/*4.选择文件夹，创建新部件工件*/

	char dirName[100];
	sprintf(dirName, "C:\\Users\\72707\\Desktop\\F%.2f%s\\", d ,isCircle == 5?"圆形":"方形");
	_mkdir(dirName);
	char partName[100];
	strcpy(partName, dirName);
	strcat(partName, "mirror.prt");
	tag_t partMirrorTag = NULL_TAG;
	UF_PART_new(partName,units,&partMirrorTag);

	/*5.在工件中建模*/

	tag_t dTag, pTag, hTag, rTag, aTag, bTag, xTag, zTag, tTag;

	char dChar[20], pChar[20], hChar[20], rChar[20], aChar[20], bChar[20];

	sprintf(dChar, "d=%.5f", d);
	sprintf(pChar, "p=%.5f", p);
	sprintf(hChar, "h=%.5f", h);
	sprintf(rChar, "r=%.5f", data[3]);
	sprintf(aChar, "a=%.5f", data[3]);
	sprintf(bChar, "b=%.5f", data[4]);

	UF_MODL_create_exp_tag(dChar, &dTag);
	UF_MODL_create_exp_tag(pChar, &pTag);
	UF_MODL_create_exp_tag(hChar, &hTag);
	UF_MODL_create_exp_tag(rChar, &rTag);
	UF_MODL_create_exp_tag(aChar, &aTag);
	UF_MODL_create_exp_tag(bChar, &bTag);
	UF_MODL_create_exp_tag("t=1", &tTag);
	UF_MODL_create_exp_tag("x=(t-0.5)*r*2+d", &xTag);
	UF_MODL_create_exp_tag("z=1/(2*p)*x^2-1/(2*p)*(d+r)^2+h", &zTag);

	tag_t cylinderTag = NULL_TAG;
	tag_t *cylinderExpTag=NULL;
	int cylinderExpNumber = 0;
	UF_free(cylinderExpTag);
	double cylinderOrgin[3] = { 0,0,0 };
	double cylinderDirection[3] = { 0,0,1 };
	cylinderOrgin[0] = d;
	UF_MODL_create_cylinder(0,NULL_TAG,cylinderOrgin,"h","2*r",cylinderDirection,&cylinderTag);
	UF_MODL_ask_exps_of_feature(cylinderTag, &cylinderExpNumber, &cylinderExpTag);

//	测试对话框
/*
	char *cylinderExp;
	UF_UI_open_listing_window();
	for (i = 0; i < cylinderExpNumber; i++)
	{
		UF_MODL_ask_exp_tag_string(cylinderExpTag[i], &cylinderExp);
		UF_UI_write_listing_window(cylinderExp);
		UF_UI_write_listing_window("\n");
	}
*/

	
	/*6.创建新部件，工装*/

	/*7.在工装中建模*/

	/*8.创建新部件粗车工件*/

	/*9.在粗车工装中建模*/

	/*10.新建图纸并出图*/

    /* Terminate the API environment */
    UF_CALL(UF_terminate());
}

/*****************************************************************************
**  Utilities
*****************************************************************************/

/* Unload Handler
**     This function specifies when to unload your application from Unigraphics.
**     If your application registers a callback (from a MenuScript item or a
**     User Defined Object for example), this function MUST return
**     "UF_UNLOAD_UG_TERMINATE". */
extern int ufusr_ask_unload( void )
{
    return( UF_UNLOAD_IMMEDIATELY );
}

