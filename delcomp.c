/*********************************************************************
* 	�Ԉ����t���[���⊮�v���O�C��
*********************************************************************/
#include <windows.h>
#include "filter.h"


static void Mul_frame(PIXEL_YC *frame,PIXEL_YC *del,int strength,int w,int h,int dif_w);



//----------------------------
//	FILTER_DLL�\����
//----------------------------
#define track_N 1
TCHAR *track_name[]   = { "���x" };	// �g���b�N�o�[�̖��O
int   track_default[] = {  50 };	// �g���b�N�o�[�̏����l
int   track_s[]       = {   0 };	// �g���b�N�o�[�̉����l
int   track_e[]       = { 100 };	// �g���b�N�o�[�̏���l
#define check_N 3
TCHAR *check_name[]   = { "�O�t���[���ɏd�˂�",
						  "��t���[���ɏd�˂�",
						  "�L�[�t���[���ɂ͏d�˂Ȃ�" };	// �`�F�b�N�{�b�N�X
int   check_default[] = { 0,1 };	// �f�t�H���g

#define tSTRENGTH  0
#define cBEFORE    0
#define cFOLLOW    1
#define cKEY       2

// �`�F�b�N�{�b�N�X�X�V�p
int check_flg = cFOLLOW;

FILTER_DLL filter = {
	FILTER_FLAG_EX_INFORMATION,	//	�t�B���^�̃t���O
	NULL,NULL,			// �ݒ�E�C���h�E�̃T�C�Y
	"�Ԉ����t���[���⊮",		// �t�B���^�̖��O
	track_N,        	// �g���b�N�o�[�̐�
	track_name,     	// �g���b�N�o�[�̖��O�S
	track_default,  	// �g���b�N�o�[�̏����l�S
	track_s,track_e,	// �g���b�N�o�[�̐��l�̉������
	check_N,      	// �`�F�b�N�{�b�N�X�̐�
	check_name,   	// �`�F�b�N�{�b�N�X�̖��O�S
	check_default,	// �`�F�b�N�{�b�N�X�̏����l�S
	func_proc,   	// �t�B���^�����֐�
	NULL,NULL,   	// �J�n��,�I�����ɌĂ΂��֐�
	NULL,        	// �ݒ肪�ύX���ꂽ�Ƃ��ɌĂ΂��֐�
	func_WndProc,	// �ݒ�E�B���h�E�v���V�[�W��
	NULL,NULL,   	// �V�X�e���Ŏg�p
	NULL,NULL,     	// �g���f�[�^
	"�Ԉ����t���[���⊮ ver0.01 by MakKi",	// �t�B���^���
	NULL,			// �Z�[�u�J�n���O�ɌĂ΂��֐�
	NULL,			// �Z�[�u�I�����ɌĂ΂��֐�
	NULL,NULL,NULL,	// �V�X�e���Ŏg�p
	NULL,			// �g���̈揉���l
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
*	�E�B���h�E�v���V�[�W��
*===================================================================*/
BOOL func_WndProc(HWND hwnd,UINT message,WPARAM wparam,LPARAM lparam,void *editp,FILTER *fp)
{
	switch(message){
		case WM_KEYUP:	// ���C���E�B���h�E�֑���
		case WM_KEYDOWN:
		case WM_MOUSEWHEEL:
			SendMessage(GetWindow(hwnd, GW_OWNER), message, wparam, lparam);
			break;
	}

	return FALSE;
}

/*====================================================================
*	�t�B���^�����֐�
*===================================================================*/
BOOL func_proc(FILTER *fp,FILTER_PROC_INFO *fpip)
{
	PIXEL_YC *delframe;
	FRAME_STATUS status;
	int r = 0;

	// �L�[�t���[���̎��d�˂Ȃ�
	if(fp->check[cKEY]){
		// �t���[���X�e�[�^�X�擾
		fp->exfunc->get_frame_status(fpip->editp,fpip->frame,&status);
		if(status.edit_flag & EDIT_FRAME_EDIT_FLAG_KEYFRAME)	// �L�[�t���[���t���O
			return FALSE;
	}

	// �摜�L���b�V���̈�Z�b�g
	fp->exfunc->set_ycp_filtering_cache_size(fp,fpip->w,fpip->h,3,NULL);

	// �O�t���[���ɏd�˂�
	if(fp->check[cBEFORE]){
		// ���t���[�����Ԉ�����邩
		if(!fp->exfunc->is_saveframe(fpip->editp,fpip->frame+1)){

			// ���t���[���̉摜�擾
			delframe = fp->exfunc->get_ycp_filtering_cache_ex(fp,fpip->editp,fpip->frame+1,NULL,NULL);
			if(!delframe)	// �擾���s
				return FALSE;

			// ���d��
			Mul_frame(fpip->ycp_edit,delframe,fp->track[tSTRENGTH],fpip->w,fpip->h,fpip->max_w-fpip->w);

			r++;
		}
	}

	// ��t���[���ɏd�˂�
	if(fp->check[cFOLLOW]){
		// �O�t���[�����Ԉ�����邩
		if(!fp->exfunc->is_saveframe(fpip->editp,fpip->frame-1)){

			// �O�t���[���̉摜�擾
			delframe = fp->exfunc->get_ycp_filtering_cache_ex(fp,fpip->editp,fpip->frame-1,NULL,NULL);
			if(!delframe)	// �擾���s
				return FALSE;

			// ���d��
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
			// ���d��
			pic->y  = (pic->y *_str + del->y *str) / 100;
			pic->cb = (pic->cb*_str + del->cb*str) / 100;
			pic->cr = (pic->cr*_str + del->cr*str) / 100;

			pic++;
			del++;
		}
		pic += dif_w;
	}
}



