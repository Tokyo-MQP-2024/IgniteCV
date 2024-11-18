#include<stdio.h>
#include<string.h>

#define WIDTH  1024
#define HEIGHT 896
#define HEADER 0
#define DATASIZE WIDTH*HEIGHT*2//撮影した画像の大きさ入れておく(データ数合わせるためここ二倍？

#define THRESHOLD 130

#define START_FRAME 1//ファイルの解析したい１番最初の数字
#define FRAME_NO 1//何枚解析するか
int pict_read(char file_name[], unsigned char* pict);
int pict_save(char file_name[], unsigned char* pict);
int binalization1(unsigned char* pict1, unsigned char* pict2);

int main()
{
	char flist_name[128], file_name[128] = { 0 };
	char f_name1[128], f_name2[128];
	char f_number[8];
	static unsigned char pict1[DATASIZE], pict2[DATASIZE];
	int frno;
	FILE* ifp;
	errno_t err;


	strcpy_s(flist_name, 128, "SampleFile.txt");/*ソースコードが保存されているファイルにおいておく,\\二個つける,ファイル名をtxtに書き込む*/
	err = fopen_s(&ifp, flist_name, "rt");
	if (err != 0) {
		printf("ファイルのオープンに失敗しました。1\n");
		return -1;
	}

	while (1)
	{
		fscanf_s(ifp, "%s", file_name, 128);

		printf_s("file name1=%s \n", file_name);


		for (frno = START_FRAME; frno < START_FRAME + FRAME_NO;frno++)
		{
			strcpy_s(f_name1, file_name);

			sprintf_s(f_number, "%d", frno);
			strcat_s(f_name1, f_number);
			strcpy_s(f_name2, f_name1);
			strcat_s(f_name1, ".raww");
			strcat_s(f_name2, "bina.raww");
			printf_s("now binalizing'%s'\n--->'%s'\n", f_name1, f_name2);

			if (pict_read(f_name1, pict1) != 0) continue;
			//二値化処理pict1--->pict2
			binalization1(pict1, pict2);
			//画像のセーブ
			pict_save(f_name2, pict2);
		}

		if (feof(ifp)) break;//ファイル名に空欄をいれない!!!!!

	}
	return 0;
}

int pict_read(char file_name[], unsigned char* pict)//pict1の読み込み
{
	int i, fgc;
	FILE* ifp;
	errno_t err;

	printf_s("now pict_read!!\n");
	printf_s("file_name= %s\n", file_name);
	printf_s("------------------------------------------ \n");

	err = fopen_s(&ifp, file_name, "rb");
	if (err != 0) {
		printf("ファイルのオープンに失敗しました。2\n");//開きたいファイルをx64があるところに入れておく
		return -1;
	}


	fseek(ifp, HEADER, 0);


	for (i = 0; i < DATASIZE; i++) {
		if ((fgc = fgetc(ifp)) == EOF) break;
		pict[i] = (unsigned char)fgc;
	}

	fclose(ifp); if (i < DATASIZE) return -2;
	return 0;

}

//画像データのセーブ.pict2-- > file_name[]
int pict_save(char file_name[], unsigned char* pict)
{
	long i;
	FILE* ofp;
	errno_t err;

	err = fopen_s(&ofp, file_name, "wb");
	if (err != 0)
	{
		printf("ファイルのオープンに失敗しました。3\n");
		return 1;/*rawデータが読み込めないと-1が帰ってくる*/
	}
	for (i = 0; i < DATASIZE; i++) {
		if ((fputc((int)pict[i], ofp)) == EOF) break;
	}
	fclose(ofp);
	if (i < DATASIZE) return -2;
	return 0;
}



//一律なしきい値の二値化処理
int binalization1(unsigned char* pict1, unsigned char* pict2)
{
	long	i;
	for (i = 0; i < DATASIZE; i++) {
		if (pict1[i] >= THRESHOLD)//２値化したのは8bit,もとのデータは16bit
			pict2[i] = 255;
		else
			pict2[i] = 0;
	}
	return 0;
}