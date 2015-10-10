/*********************************************************************
* 	間引きフレーム補完プラグイン
*********************************************************************/
#include <windows.h>
#include "filter.h"


static void Mul_frame(PIXEL_YC *frame,PIXEL_YC *del,int strength,int w,int h,int dif_w);



//----------------------------
//	FILTER_DLL構造体
//----------------------------
#define track_N 1
TCHAR *track_name[]   = { "強度" };	// トラックバーの名前
int   track_default[] = {  50 };	// トラックバーの初期値
int   track_s[]       = {   0 };	// トラックバーの下限値
int   track_e[]       = { 100 };	// トラックバーの上限値
#define check_N 3
TCHAR *check_name[]   = { "前フレームに重ねる",
						  "後フレームに重ねる",
						  "キーフレームには重ねない" };	// チェックボックス
int   check_default[] = { 0,1 };	// デフォルト

#define tSTRENGTH  0
#define cBEFORE    0
#define cFOLLOW    1
#define cKEY       2

// チェックボックス更新用
int check_flg = cFOLLOW;

FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION,	//	フィルタのフラグ
	NULL,NULL,			// 設定ウインドウのサイズ
	"間引きフレーム補完",		// フィルタの名前
	track_N,        	// トラックバーの数
	track_name,     	// トラックバーの名前郡
	track_default,  	// トラックバーの初期値郡
	track_s,track_e,	// トラックバーの数値の下限上限
	check_N,      	// チェックボックスの数
	check_name,   	// チェックボックスの名前郡
	check_default,	// チェックボックスの初期値郡
	func_proc,   	// フィルタ処理関数
	NULL,NULL,   	// 開始時,終了時に呼ばれる関数
	NULL,        	// 設定が変更されたときに呼ばれる関数
	func_WndProc,	// 設定ウィンドウプロシージャ
	NULL,NULL,   	// システムで使用
	NULL,NULL,     	// 拡張データ
	"間引きフレーム補完 ver0.01 by MakKi",	// フィルタ情報
	NULL,			// セーブ開始直前に呼ばれる関数
	NULL,			// セーブ終了時に呼ばれる関数
	NULL,NULL,NULL,	// システムで使用
	NULL,			// 拡張領域初期値
	NULL,
	NULL
};

/*********************************************************************
*	DLL Export
*********************************************************************/
EXTERN_C FILTER_DLL __declspec(dllexport) * __stdcall GetFilterTable( void )
{
	return &filter;
}

/*====================================================================
*	ウィンドウプロシージャ
*===================================================================*/
BOOL func_WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *fp)
{
	switch(message){
		case WM_KEYUP:	// メインウィンドウへ送る
		case WM_KEYDOWN:
		case WM_MOUSEWHEEL:
			SendMessage(GetWindow(hwnd, GW_OWNER), message, wparam, lparam);
			break;
	}

	return FALSE;
}

/*====================================================================
*	フィルタ処理関数
*===================================================================*/
BOOL func_proc(FILTER *fp,FILTER_PROC_INFO *fpip)
{
	PIXEL_YC *delframe;
	FRAME_STATUS status;
	int r = 0;

	// キーフレームの時重ねない
	if(fp->check[cKEY]){
		// フレームステータス取得
		fp->exfunc->get_frame_status(fpip->editp,fpip->frame,&status);
		if(status.edit_flag & EDIT_FRAME_EDIT_FLAG_KEYFRAME)	// キーフレームフラグ
			return FALSE;
	}

	// 画像キャッシュ領域セット
	fp->exfunc->set_ycp_filtering_cache_size(fp,fpip->w,fpip->h,3,NULL);

	// 前フレームに重ねる
	if(fp->check[cBEFORE]){
		// 次フレームが間引かれるか
		if(!fp->exfunc->is_saveframe(fpip->editp,fpip->frame+1)){

			// 次フレームの画像取得
			delframe = fp->exfunc->get_ycp_filtering_cache_ex(fp,fpip->editp,fpip->frame+1,NULL,NULL);
			if(!delframe)	// 取得失敗
				return FALSE;

			// 多重化
			Mul_frame(fpip->ycp_edit,delframe,fp->track[tSTRENGTH],fpip->w,fpip->h,fpip->max_w-fpip->w);

			r++;
		}
	}

	// 後フレームに重ねる
	if(fp->check[cFOLLOW]){
		// 前フレームが間引かれるか
		if(!fp->exfunc->is_saveframe(fpip->editp,fpip->frame-1)){

			// 前フレームの画像取得
			delframe = fp->exfunc->get_ycp_filtering_cache_ex(fp,fpip->editp,fpip->frame-1,NULL,NULL);
			if(!delframe)	// 取得失敗
				return FALSE;

			// 多重化
			Mul_frame(fpip->ycp_edit,delframe,fp->track[tSTRENGTH],fpip->w,fpip->h,fpip->max_w-fpip->w);

			r++;
		}
	}

	return r;
}


static void Mul_frame(PIXEL_YC *pic,PIXEL_YC *del,int str,int w,int h,int dif_w)
{
	int _str;
	int i;

	_str  = 100 - str;

	for(;h>0;h--){
		for(i=w;i>0;i--){
			// 多重化
			pic->y  = (pic->y *_str + del->y *str) / 100;
			pic->cb = (pic->cb*_str + del->cb*str) / 100;
			pic->cr = (pic->cr*_str + del->cr*str) / 100;

			pic++;
			del++;
		}
		pic += dif_w;
	}
}



