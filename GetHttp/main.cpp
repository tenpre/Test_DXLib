#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>

#include <DxLib.h>

const int BufSize = 1024;									// バッファサイズ


const TCHAR serverName[] = TEXT("homepage2.nifty.com");		// 接続先ホスト名
const TCHAR url[] = TEXT("/natupaji/DxLib/");				// 取得するファイル位置(DXライブラリ公式)
//const TCHAR url[] = TEXT("/");								// ルート(index)ならこのようにする

const short Port = 80;						// HTTPのポート番号
const int recieveLoopMax = 10000;			// 受信ループの最大回数
const int updateCount = 3;					// 読み取り文字数の更新しきい値

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	LPSTR lpCmdLine, int nCmdShow)
{
	IPDATA ipAddress;                   // 接続用ＩＰアドレスデータ
	int netHandle;               // ネットワークハンドル

	// データバッファ
	char strBuf[BufSize] = { 0 };

	ChangeWindowMode(TRUE);
	if (DxLib_Init() == -1)
	{
		return -1;
	}

	// DXライブラリの通信は独自のプロトコルが混じってるらしいので、FALSEでやめさせる
	// この場合、通信関連の関数の動作がかなり変化するので気をつけること
	// 参考：http://hpcgi2.nifty.com/natupaji/bbs/patio.cgi?mode=past&no=736&p=2
	SetUseDXNetWorkProtocol(FALSE);

	// サーバ名からIPアドレスを取得
	GetHostIPbyName(serverName, &ipAddress);

	// 通信を確立
	netHandle = ConnectNetWork(ipAddress, Port);

	// 確立が成功した場合のみ中の処理をする
	if (netHandle != -1)
	{
		int sendLength = 0;
		int readLength = 0;
		int isDataReadable;
		int requireLength = 4;

		// データ送信
		sprintf_s(strBuf, BufSize, "GET %s  HTTP/1.0\nHost: %s\n\n", url, serverName);
		sendLength = strnlen_s(strBuf, BufSize);
		NetWorkSend(netHandle, strBuf, sendLength + 1);

		// 受信したデータをファイルに書き出す
		std::ofstream ofs("output.txt", std::ios::trunc);

		int loopCount = 0;
		int successCount = 0;

		// データが読み終わるまで待機
		while (!ProcessMessage())
		{
			// 一定のループ回数に達したら強制終了
			if (loopCount++ >= 10000)
				break;
			// 取得していない受信データがあるかチェック
			isDataReadable = GetNetWorkDataLength(netHandle);

			// 取得してない受信データ量がない場合はループを抜ける
			if (isDataReadable == FALSE) break;

			// データ受信し、読み込んだ分をサイズに増やす
			readLength = NetWorkRecv(netHandle, strBuf, requireLength);
			
			// 読み取りエラー
			// 待機時の他にバッファ内に溜まっている文字数よりも requireLength が大きい時も -1 になるので注意
			if (readLength == -1)
			{
				if (--successCount <= -updateCount)
				{
					if (--requireLength == 0)
						requireLength = 1;
					successCount = 0;
					// バッファに溜まるように適当に待機
					WaitTimer(1000);
				}
				continue;
			}
			// 規定回数読み取りが成功したら読み取り文字数を増やす
			if (++successCount >= updateCount)
			{
				++requireLength;
				successCount = 0;
			}

			// 末端文字を付加
			strBuf[readLength] = '\0';
			// 受信したデータをファイルに出力
			ofs << strBuf;
			printfDx("%d: %s\n", loopCount, strBuf);

			ScreenFlip();
			ClearDrawScreen();
		}

		// 接続を断つ
		CloseNetWork(netHandle);

		ofs.flush();
		ofs.close();

		// キー入力待ち
		printfDx("\nPlease Push Any Key...");
		WaitKey();
	}

	DxLib_End();

	return 0;
}
