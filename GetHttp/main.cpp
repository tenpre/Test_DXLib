#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <DxLib.h>

const int BufSize = 1024;									// �o�b�t�@�T�C�Y


const TCHAR serverName[] = TEXT("homepage2.nifty.com");		// �ڑ���z�X�g��
const TCHAR url[] = TEXT("/natupaji/DxLib/");				// �擾����t�@�C���ʒu(DX���C�u��������)
//const TCHAR url[] = TEXT("/");								// ���[�g(index)�Ȃ炱�̂悤�ɂ���

const short Port = 80;						// HTTP�̃|�[�g�ԍ�
const int recieveLoopMax = 10000;			// ��M���[�v�̍ő��
const int updateCount = 3;					// �ǂݎ�蕶�����̍X�V�������l

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	IPDATA ipAddress;                   // �ڑ��p�h�o�A�h���X�f�[�^
	int netHandle;               // �l�b�g���[�N�n���h��

	// �f�[�^�o�b�t�@
	char strBuf[BufSize] = { 0 };

	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1)
	{
		return -1;
	}

	// DX���C�u�����̒ʐM�͓Ǝ��̃v���g�R�����������Ă�炵���̂ŁAFALSE�ł�߂�����
	// ���̏ꍇ�A�ʐM�֘A�̊֐��̓��삪���Ȃ�ω�����̂ŋC�����邱��
	// �Q�l�Fhttp://hpcgi2.nifty.com/natupaji/bbs/patio.cgi?mode=past&no=736&p=2
	SetUseDXNetWorkProtocol(FALSE);

	// �T�[�o������IP�A�h���X���擾
	GetHostIPbyName(serverName, &ipAddress);

	// �ʐM���m��
	netHandle = ConnectNetWork(ipAddress, Port);

	// �m�������������ꍇ�̂ݒ��̏���������
	if (netHandle != -1)
	{
		int sendLength = 0;
		int readLength = 0;
		int isDataReadable;
		int requireLength = 4;

		// �f�[�^���M
		sprintf_s(strBuf, BufSize, "GET %s  HTTP/1.0\nHost: %s\n\n", url, serverName);
		sendLength = strnlen_s(strBuf, BufSize);
		NetWorkSend(netHandle, strBuf, sendLength + 1);

		// ��M�����f�[�^���t�@�C���ɏ����o��
		std::ofstream ofs("output.txt", std::ios::trunc);

		int loopCount = 0;
		int successCount = 0;

		// �f�[�^���ǂݏI���܂őҋ@
		while (!ProcessMessage())
		{
			// ���̃��[�v�񐔂ɒB�����狭���I��
			if (loopCount++ >= 10000)
				break;
			// �擾���Ă��Ȃ���M�f�[�^�����邩�`�F�b�N
			isDataReadable = GetNetWorkDataLength(netHandle);

			// �擾���ĂȂ���M�f�[�^�ʂ��Ȃ��ꍇ�̓��[�v�𔲂���
			if (isDataReadable == FALSE) break;

			// �f�[�^��M���A�ǂݍ��񂾕����T�C�Y�ɑ��₷
			readLength = NetWorkRecv(netHandle, strBuf, requireLength);
			
			// �ǂݎ��G���[
			// �ҋ@���̑��Ƀo�b�t�@���ɗ��܂��Ă��镶�������� requireLength ���傫������ -1 �ɂȂ�̂Œ���
			if (readLength == -1)
			{
				if (--successCount <= -updateCount)
				{
					if (--requireLength == 0)
						requireLength = 1;
					successCount = 0;
					// �o�b�t�@�ɗ��܂�悤�ɓK���ɑҋ@
					WaitTimer(1000);
				}
				continue;
			}
			// �K��񐔓ǂݎ�肪����������ǂݎ�蕶�����𑝂₷
			if (++successCount >= updateCount)
			{
				++requireLength;
				successCount = 0;
			}

			// ���[������t��
			strBuf[readLength] = '\0';
			// ��M�����f�[�^���t�@�C���ɏo��
			ofs << strBuf;
			printfDx("%d: %s\n", loopCount, strBuf);

			ScreenFlip();
			ClearDrawScreen();
		}

		// �ڑ���f��
		CloseNetWork(netHandle);

		ofs.flush();
		ofs.close();

		// �L�[���͑҂�
		printfDx("\nPlease Push Any Key...");
		WaitKey();
	}

	DxLib_End();

	return 0;
}
