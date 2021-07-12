
// RGBDlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "RGB.h"
#include "RGBDlg.h"
#include "afxdialogex.h"
#include "afxtempl.h"
#include "atltypes.h"
#include <cmath>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif
void chageColor(Mat img, Mat &copy, int i);
Mat dilation(Mat img);
Mat erosion(Mat img);
Mat closing(Mat img);
Mat opening(Mat img);
void contourTracing(Mat& m, CArray<CPoint, CPoint>& contours);
void train(int fig_num, CArray<CPoint, CPoint>&, CArray<double, double&>&);
void lcs_extract(CArray<CPoint, CPoint>& c, CArray<double, double&>& lcs);
void morp(Mat& img);
void segmentation(Mat img);
Mat grayscale(Mat img);
double DTW(CArray<double, double&>& test_lcs, CArray<double, double&>& ref_lcs);

Mat closing_img;
Mat opening_img;
Mat m;
Mat labImg; 
Mat labImg2;
int structuring_element[5][5] = { {255,255,255,255,255},{255,255,255,255,255},{255,255,255,255,255},{255,255,255,255,255},{255,255,255,255,255} };
int dy[25] = { -2,-2,-2,-2,-2,-1,-1,-1,-1,-1,0,0,0,0,0,1,1,1,1,1,2,2,2,2,2 };
int dx[25] = { -2,-1,0,1,2,-2,-1,0,1,2,-2,-1,0,1,2 ,-2,-1,0,1,2 ,-2,-1,0,1,2 };
int LUT_BLabeling4[4][4] = { {0,0,0,0},{0,0,0,1}, {1,0,0,1}, {1,1,0,1} };
int LUT_BLabeling8[8][8] = { {0,0,0,0,0,0,0,0}, {0,0,0,0,0,1,0,0},{0,0,0,0,0,1,1,0},
	{0,0,0,0,0,1,1,1}, {1,0,0,0,0,1,1,1}, {1,1,0,0,0,1,1,1},{1,1,1,0,0,1,1,1},{1,1,1,1,0,1,1,1} };
int num_region[500] = { 0, };
CArray<CPoint, CPoint> contour_array;
CArray<double, double&> lcs_array;

CArray <CPoint, CPoint> chex;
CArray <CPoint, CPoint> crect;
CArray <CPoint, CPoint> ccircle;

CArray<double, double&>	hex_lcs;
CArray<double, double&> rect_lcs;
CArray<double, double&> circle_lcs;

int dy_dtw[3]={-1,0,-1};
int dx_dtw[3] = { 0,-1,-1 };

#define FOREWARD 1
#define BACKWARD 2
#define INF 987654321

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRGBDlg 대화 상자



CRGBDlg::CRGBDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_RGB_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRGBDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_Img, m_pic);
}

BEGIN_MESSAGE_MAP(CRGBDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_Img_Search, &CRGBDlg::OnBnClickedImgSearch)
	//ON_BN_CLICKED(IDC_Img_Save, &CRGBDlg::OnBnClickedImgSave)
	ON_BN_CLICKED(IDC_Red_Btn, &CRGBDlg::OnBnClickedRedBtn)
	ON_BN_CLICKED(IDC_Green_Btn, &CRGBDlg::OnBnClickedGreenBtn)
	ON_BN_CLICKED(IDC_Blue_Btn, &CRGBDlg::OnBnClickedBlueBtn)
	ON_BN_CLICKED(IDC_Gray_Btn, &CRGBDlg::OnBnClickedGrayBtn)
	ON_BN_CLICKED(IDC_Btn_Otsu, &CRGBDlg::OnBnClickedBtnOtsu)
	ON_BN_CLICKED(IDOK, &CRGBDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_DILATION, &CRGBDlg::OnBnClickedDilation)
	//ON_BN_CLICKED(IDC_BUTTON4, &CRGBDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_EROSION, &CRGBDlg::OnBnClickedErosion)
	ON_BN_CLICKED(IDC_CLOSING, &CRGBDlg::OnBnClickedClosing)
	ON_BN_CLICKED(IDC_Opening, &CRGBDlg::OnBnClickedOpening)
	
	ON_BN_CLICKED(IDC_Training1, &CRGBDlg::OnBnClickedTraining1)
	ON_BN_CLICKED(IDC_Testing, &CRGBDlg::OnBnClickedTesting)
	ON_BN_CLICKED(IDC_Conturing, &CRGBDlg::OnBnClickedConturing)
	ON_BN_CLICKED(IDC_LCS, &CRGBDlg::OnBnClickedLcs)
	ON_BN_CLICKED(IDC_Matching, &CRGBDlg::OnBnClickedMatching)
END_MESSAGE_MAP()


// CRGBDlg 메시지 처리기

BOOL CRGBDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CRGBDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CRGBDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CRGBDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRGBDlg::OnBnClickedImgSearch()
{
	static TCHAR BASED_CODE szFilter[] = _T("이미지 파일(*.BMP, *.GIF, *.JPG) | *.BMP;*.GIF;*.JPG;*.bmp;*.jpg;*.gif |모든파일(*.*)|*.*||");
	CFileDialog dlg(TRUE, _T("*.jpg"), _T("image"), OFN_HIDEREADONLY, szFilter);
	if (IDOK == dlg.DoModal())
	{
		pathName = dlg.GetPathName();
		CT2CA pszConvertedAnsiString_up(pathName);
		std::string up_pathName_str(pszConvertedAnsiString_up);
		img = cv::imread(up_pathName_str);
		DisplayImage(img, 3);
	}
}
void CRGBDlg::DisplayImage(Mat targetMat, int channel)
{
	CDC *pDC = nullptr;
	CImage* mfcImg = nullptr;

	pDC = m_pic.GetDC();
	CStatic *staticSize = (CStatic *)GetDlgItem(IDC_Img);
	staticSize->GetClientRect(rect);

	cv::Mat tempImage;
	cv::resize(targetMat, tempImage, Size(rect.Width(), rect.Height()));

	BITMAPINFO bitmapInfo;
	bitmapInfo.bmiHeader.biYPelsPerMeter = 0;
	bitmapInfo.bmiHeader.biBitCount = 24;
	bitmapInfo.bmiHeader.biWidth = tempImage.cols;
	bitmapInfo.bmiHeader.biHeight = tempImage.rows;
	bitmapInfo.bmiHeader.biPlanes = 1;
	bitmapInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfo.bmiHeader.biCompression = BI_RGB;
	bitmapInfo.bmiHeader.biClrImportant = 0;
	bitmapInfo.bmiHeader.biClrUsed = 0;
	bitmapInfo.bmiHeader.biSizeImage = 0;
	bitmapInfo.bmiHeader.biXPelsPerMeter = 0;

	if (targetMat.channels() == 3)
	{
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 24);
	}
	else if (targetMat.channels() == 1)
	{
		cvtColor(tempImage, tempImage, CV_GRAY2RGB);
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 24);
	}
	else if (targetMat.channels() == 4)
	{
		bitmapInfo.bmiHeader.biBitCount = 32;
		mfcImg = new CImage();
		mfcImg->Create(tempImage.cols, tempImage.rows, 32);
	}
	cv::flip(tempImage, tempImage, 0);
	::StretchDIBits(mfcImg->GetDC(), 0, 0, tempImage.cols, tempImage.rows,
		0, 0, tempImage.cols, tempImage.rows, tempImage.data, &bitmapInfo,
		DIB_RGB_COLORS, SRCCOPY);

	mfcImg->BitBlt(::GetDC(m_pic.m_hWnd), 0, 0);

	if (mfcImg)
	{
		mfcImg->ReleaseDC();
		delete mfcImg;// mfcImg = nullptr;
	}
	tempImage.release();
	ReleaseDC(pDC);
}

void CRGBDlg::OnBnClickedRedBtn()
{
	Mat img_copy = img.clone();
	chageColor(img, img_copy, 1);
	DisplayImage(img_copy, 3);
	img = img_copy.clone();
}


void CRGBDlg::OnBnClickedGreenBtn()
{
	Mat img_copy = img.clone();
	chageColor(img, img_copy, 2);
	DisplayImage(img_copy, 3);
	img = img_copy.clone();
}


void CRGBDlg::OnBnClickedBlueBtn()
{
	Mat img_copy = img.clone();
	chageColor(img, img_copy, 3);
	DisplayImage(img_copy, 3);
	img = img_copy.clone();
}

void CRGBDlg::OnBnClickedGrayBtn()
{
	Mat gray_img(img.rows, img.cols, CV_8UC1);
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			gray_img.at<uchar>(y, x) = (img.at<Vec3b>(y, x)[0] + img.at<Vec3b>(y, x)[1] + img.at<Vec3b>(y, x)[2]) / 3;
		}
	}
	DisplayImage(gray_img, 1);
	img = gray_img.clone();
}

Mat grayscale(Mat img) {
	Mat gray_img(img.rows, img.cols, CV_8UC1);
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			gray_img.at<uchar>(y, x) = (img.at<Vec3b>(y, x)[0] + img.at<Vec3b>(y, x)[1] + img.at<Vec3b>(y, x)[2]) / 3;
		}
	}
	return gray_img.clone();
}

void CRGBDlg::OnBnClickedBtnOtsu()
{
	double H[256] = { 0, };
	double NH[256] = { 0, };
	double w0[256]; double m0[256]; double m1[256];
	double v_between[256] = { 0, };
	double m = 0;
	double max_value = 0;
	int T = 0;
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			H[img.at<uchar>(y, x)]++;
		}
	}
	for (int i = 0; i < 256; i++) {
		NH[i] = H[i] / (img.rows * img.cols);
	}
	for (int i = 0; i < 256; i++) {
		m += (i * NH[i]);
	}
	w0[0] = NH[0];
	m0[0] = 0;
	for (int t = 1; t < 256; t++) {
		w0[t] = w0[t - 1] + NH[t];
		if (w0[t] == 0) {
			m0[t] = 0;
		}
		else {
			m0[t] = (w0[t - 1] * m0[t - 1] + (t * NH[t])) / w0[t];
		}
		m1[t] = (m - (w0[t] * m0[t])) / (1 - w0[t]);
		v_between[t] = w0[t] * (1 - w0[t]) * (m0[t] - m1[t]) * (m0[t] - m1[t]);
		if (max_value < v_between[t]) {
			T = t;
			max_value = v_between[t];
		}
	}
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			if (img.at<uchar>(y, x) >= T) {
				img.at<uchar>(y, x) = 255;
			}
			else {
				img.at<uchar>(y, x) = 0;
			}
		}
	}
	DisplayImage(img, 1);
}

void segmentation(Mat img) {
	double H[256] = { 0, };
	double NH[256] = { 0, };
	double w0[256]; double m0[256]; double m1[256];
	double v_between[256] = { 0, };
	double m = 0;
	double max_value = 0;
	int T = 0;
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			H[img.at<uchar>(y, x)]++;
		}
	}
	for (int i = 0; i < 256; i++) {
		NH[i] = H[i] / (img.rows * img.cols);
	}
	for (int i = 0; i < 256; i++) {
		m += (i * NH[i]);
	}
	w0[0] = NH[0];
	m0[0] = 0;
	for (int t = 1; t < 256; t++) {
		w0[t] = w0[t - 1] + NH[t];
		if (w0[t] == 0) {
			m0[t] = 0;
		}
		else {
			m0[t] = (w0[t - 1] * m0[t - 1] + (t * NH[t])) / w0[t];
		}
		m1[t] = (m - (w0[t] * m0[t])) / (1 - w0[t]);
		v_between[t] = w0[t] * (1 - w0[t]) * (m0[t] - m1[t]) * (m0[t] - m1[t]);
		if (max_value < v_between[t]) {
			T = t;
			max_value = v_between[t];
		}
	}
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			if (img.at<uchar>(y, x) >= T) {
				img.at<uchar>(y, x) = 255;
			}
			else {
				img.at<uchar>(y, x) = 0;
			}
		}
	}
}


void CRGBDlg::OnBnClickedConturing()
{
	contour_array.RemoveAll();
	contourTracing(img, contour_array);
	DisplayImage(labImg2, 1);
}


void read_neighbor8(int y, int x, int *neighbor8, Mat &m) {
	int dy[8] = { 0,1,1,1,0,-1,-1,-1 };
	int dx[8] = { 1,1,0,-1,-1,-1,0,1 };
	int rows = m.rows;
	int cols = m.cols;
	int pos_y, pos_x;
	for (int i = 0; i < 8; i++) {
		pos_y = y + dy[i];
		pos_x = x + dx[i];
		if (pos_y < 0 || pos_y >= rows || pos_x < 0 || pos_x >= cols) {
			neighbor8[i] = 0;
			continue;
		}
		neighbor8[i] = m.at<uchar>(pos_y, pos_x);
	}
}

void calCoord(int i, int* y, int* x) {
	switch (i) {
	case 0:
		*x = *x + 1;
		break;
	case 1:
		*y = *y + 1;
		*x = *x + 1;
		break;
	case 2:
		*y = *y + 1;
		break;
	case 3:
		*y = *y + 1;
		*x = *x - 1;
		break;
	case 4:
		*x = *x - 1;
		break;
	case 5:
		*y = *y - 1;
		*x = *x - 1;
		break;
	case 6:
		*y = *y - 1;
		break;
	case 7:
		*y = *y - 1;
		*x = *x + 1;
		break;
	}
}


void BTracing(int y, int x, int label, int tag, CArray<CPoint, CPoint> &contours, Mat &m) {
	int cur_orient, pre_orient, end_x, pre_x, end_y, pre_y;
	int start_o, add_o;
	int neighbor8[8];
	int i;
	if (tag == FOREWARD) {
		cur_orient = pre_orient = 0;
	}
	else {
		cur_orient = pre_orient = 6;
	}
	end_x = pre_x = x;
	end_y = pre_y = y;
	do {
		read_neighbor8(y, x, neighbor8, m);
		start_o = (8 + cur_orient - 2) % 8;
		for (i = 0; i < 8; i++) {
			add_o = (start_o + i) % 8;
			if (neighbor8[add_o] != 0) {
				break;
			}
		}
		if (i < 8) {
			calCoord(add_o, &y, &x);
			cur_orient = add_o;
		}
		if (LUT_BLabeling8[pre_orient][cur_orient]==1) {
			num_region[label]++;
			labImg.at<uchar>(pre_y, pre_x) = label;
			labImg2.at<uchar>(pre_y, pre_x) = 255;
			contours.Add(CPoint(x, y));
		}
		pre_x = x; 
		pre_y = y;
		pre_orient = cur_orient;
	} while ((y != end_y) || (x != end_x));
}



void contourTracing(Mat &m, CArray<CPoint, CPoint>& contours) {
	int labelnumber = 1;
	int cur_p, ref_p1, ref_p2;
	int y, x;
	labImg=Mat::zeros(m.rows, m.cols, CV_8UC1);
	labImg2 = Mat::zeros(m.rows, m.cols, CV_8UC1);
	for (y = 1; y < (m.rows-1); y++) {
		for (x = 1; x < (m.cols - 1); x++) {
			cur_p = m.at<uchar>(y, x);
			if (cur_p == 255 && labImg.at<uchar>(y,x)==0) {		 
				ref_p1 = labImg.at<uchar>(y, x - 1);
				ref_p2 = labImg.at<uchar>(y - 1, x - 1);
				if (ref_p1 > 1) {
					num_region[ref_p1]++;
					labImg.at<uchar>(y, x) = ref_p1;
					//labImg2.at<uchar>(y, x) = 255;
				}
				else if ((ref_p1 == 0) && (ref_p2 >= 2)) {
					num_region[ref_p2]++;
					labImg.at<uchar>(y, x) = ref_p2;
					labImg2.at<uchar>(y, x) = 255;
					contours.Add(CPoint(x,y));
					BTracing(y, x, ref_p2, BACKWARD, contours, m);
				}
				else if ((ref_p1 == 0) && (ref_p2 == 0)) {
					labelnumber++;
					num_region[labelnumber]++;
					labImg.at<uchar>(y, x) = labelnumber;
					labImg2.at<uchar>(y, x) = 255;
					contours.Add(CPoint(x, y));
					BTracing(y, x, labelnumber, FOREWARD, contours, m);
				}
			}
		}
	}
} 

Mat dilation(Mat img) {
	int ny, nx;
	int flag;
	Mat result(img.rows, img.cols, CV_8UC1);
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			flag = 0;
			for (int i = 0; i < 5 * 5; i++) {
				ny = y + dy[i];
				nx = x + dx[i];
				if (ny < 0 || nx < 0 || ny >= img.rows || nx >= img.cols) {
					continue;
				}
				if (structuring_element[2 + dy[i]][2 + dx[i]] == 0) {
					continue;
				}
				if (img.at<uchar>(ny, nx) == structuring_element[2 + dy[i]][2 + dx[i]]) {	//한 픽셀이라도 서로 동일한 값을 가질 경우 flag값 1로 바꾸고 반복문 탈출
					flag = 1;
					break;
				}
			}
			if (flag == 1) {
				result.at<uchar>(y, x) = 255;
			}
			else {
				result.at<uchar>(y, x) = 0;
			}
		}
	}
	return result;
}

Mat erosion(Mat img) {
	int ny, nx;
	int flag;
	Mat result(img.rows, img.cols, CV_8UC1);
	for (int y = 0; y < img.rows; y++) {
		for (int x = 0; x < img.cols; x++) {
			flag = 0;
			for (int i = 0; i < 5 * 5; i++) {
				ny = y + dy[i];
				nx = x + dx[i];
				if (ny < 0 || nx < 0 || ny >= img.rows || nx >= img.cols) {
					continue;
				}
				if (structuring_element[2 + dy[i]][2 + dx[i]] == 0) {
					continue;
				}
				if (img.at<uchar>(ny, nx) != structuring_element[2 + dy[i]][2 + dx[i]]) {	//한 픽셀이라도 서로 다른 값을 가질 경우 flag값 1로 바꾸고 반복문 탈출
					flag = 1;
					break;
				}
			}
			if (flag == 1) {
				result.at<uchar>(y, x) = 0;
			}
			else {
				result.at<uchar>(y, x) = 255;
			}
		}
	}
	return result;
}

Mat closing(Mat img) {
	Mat result;
	result = dilation(img);
	return erosion(result);
}

Mat opening(Mat img) {
	Mat result;
	result = erosion(img);
	return dilation(result);
}

void CRGBDlg::OnBnClickedDilation()
{
	img = dilation(img);
	DisplayImage(img, 1);

}

void CRGBDlg::OnBnClickedErosion()
{
	img = erosion(img);
	DisplayImage(img, 1);
}


void CRGBDlg::OnBnClickedClosing()
{
	closing_img = closing(img);
	DisplayImage(closing_img, 1);
	img = closing_img.clone();
}


void CRGBDlg::OnBnClickedOpening()
{
	opening_img = opening(img);
	DisplayImage(opening_img, 1);
	img = opening_img.clone();
}

void morp(Mat& img) {
	Mat result;
	result = closing(img);
	img = opening(result);
}



void chageColor(Mat img, Mat &copy, int i)
{
	if (i == 1)
	{
		for (int y = 0; y < copy.rows; y++)
		{
			unsigned char* ptr1 = copy.data + 3 * (copy.cols*y);
			unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
			for (int x = 0; x<copy.cols; x++)
			{
				// 이렇게 RGB값을 조정하여 그 범위 안에 있는 Rgb 픽셀값에 단색을 넣었다.
				//200 -> 160 -> 110
				//그림자
				//배경색을 초록으로 해도 결과가 바뀌므로 함부로 손대지 말자
				ptr1[3 * x + 0] = 0;
				ptr1[3 * x + 1] = 0;
				ptr1[3 * x + 2] = ptr1[3 * x + 2];
			}
		}
	}
	else if (i == 2)
	{
		for (int y = 0; y < copy.rows; y++)
		{
			unsigned char* ptr1 = copy.data + 3 * (copy.cols*y);
			unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
			for (int x = 0; x<copy.cols; x++)
			{
				// 이렇게 RGB값을 조정하여 그 범위 안에 있는 Rgb 픽셀값에 단색을 넣었다.
				//200 -> 160 -> 110
				//그림자
				//배경색을 초록으로 해도 결과가 바뀌므로 함부로 손대지 말자
				ptr1[3 * x + 0] = 0;
				ptr1[3 * x + 1] = ptr1[3 * x + 1];
				ptr1[3 * x + 2] = 0;
			}
		}
	}
	else if(i == 3)
	{
		for (int y = 0; y < copy.rows; y++)
		{
			unsigned char* ptr1 = copy.data + 3 * (copy.cols*y);
			unsigned char* resultptr = copy.data + 3 * (copy.cols*y);
			for (int x = 0; x<copy.cols; x++)
			{
				// 이렇게 RGB값을 조정하여 그 범위 안에 있는 Rgb 픽셀값에 단색을 넣었다.
				//200 -> 160 -> 110
				//그림자
				//배경색을 초록으로 해도 결과가 바뀌므로 함부로 손대지 말자
				ptr1[3 * x + 0] = ptr1[3 * x + 0];
				ptr1[3 * x + 1] = 0;
				ptr1[3 * x + 2] = 0;
			}
		}
	}

}


void CRGBDlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CDialogEx::OnOK();
}



void CRGBDlg::OnBnClickedTraining1()
{
	train(1,chex,hex_lcs);
	train(2, crect, rect_lcs);
	train(3, ccircle, circle_lcs);
	MessageBox(_T("이미지 학습 완료!"), _T(""));
}

void CRGBDlg::OnBnClickedTesting()		//Load 버튼으로 이미지 load 후 test 버튼 클릭
{
	double smallest = (double)INF;
	int fig = 0;
	double result[3];
	Mat m = img;
	CArray<CPoint, CPoint> test_contour;
	CArray<double, double&> test_lcs;
	m = grayscale(m);
	segmentation(m);
	morp(m);
	contourTracing(m, test_contour);
	DisplayImage(labImg2, 1);
	lcs_extract(test_contour, test_lcs);
	result[0]=DTW(test_lcs, hex_lcs);
	result[1]=DTW(test_lcs, rect_lcs);
	result[2]=DTW(test_lcs, circle_lcs);
	for (int i = 0; i < 3; i++) {
		if (smallest > result[i]) {
			smallest = result[i];
			fig = i + 1;
		}
	}
	switch (fig) {
	case 1:
		MessageBox(_T("이 도형은 육각형(hexagon) 입니다."), _T(""));
		break;
	case 2:
		MessageBox(_T("이 도형은 직사각형(rectangle) 입니다."), _T(""));
		break;
	case 3:
		MessageBox(_T("이 도형은 원(circle) 입니다."), _T(""));
		break;
	}
}


double DTW(CArray<double, double&> &test_lcs, CArray<double, double&> &ref_lcs) {		//dissimilarity 값 리턴하도록
	int i, j, k;
	double **D;
	int	**G;
	double S; //dissimilarity
	int n = (int)test_lcs.GetSize(); 
	int m = (int)ref_lcs.GetSize();
	D = new double* [n];
	G = new int* [n];
	double smallest;
	int direction;
	for (i = 0; i < n; i++) {
		D[i] = new double[m];
		G[i] = new int[m];
	}
	D[0][0] = fabs(test_lcs.GetAt(0) - ref_lcs.GetAt(0));
	G[0][0] = 0;
	for (j = 1; j < m; j++) {
		D[0][j] = D[0][j - 1] + fabs(test_lcs.GetAt(0) - ref_lcs.GetAt(j)); 
		G[0][j] = 2;
	}
	for (i = 1; i < n; i++) {
		D[i][0] = (double)INF;
	}
	for (i = 1; i < n; i++) {
		for (j = 1; j < m; j++) {
			D[i][j] = fabs(test_lcs.GetAt(i) - ref_lcs.GetAt(j));
			smallest = (double)INF;
			direction = 0;
			for (k = 0; k < 3; k++) {
				if (smallest > D[i + dy_dtw[k]][j + dx_dtw[k]]) {
					smallest = D[i + dy_dtw[k]][j + dx_dtw[k]];
					direction = k + 1;
				}
			}
			D[i][j] += smallest;
			G[i][j] = direction;
		}
	}
	i = n - 1;
	j = m - 1;
	k = 1;
	while ((i != 0) && (j != 0)) {
		switch (G[i][j]) {
			case 1:
				i--;
				k++;
				break;
			case 2:
				j--;
				k++;
				break;
			case 3:
				i--;
				j--;
				k++;
				break;
		}
	}
	S = D[n - 1][m - 1] / (double)k;
	for (i = 0; i < n; i++) {
		delete[] D[i];
		delete[] G[i];
	}
	delete[] D;
	delete[] G;
	return S;
}


void lcs_extract(CArray<CPoint, CPoint>& c, CArray<double, double&>& lcs) {	//w=13
	int i;
	int x, y, x1, y1, x2, y2;
	int left, right;
	int a, b, z;
	double dist;
	int totalSize = (int)c.GetSize();
	for (i = 0; i < totalSize; i++) {
		x = c.GetAt(i).x;
		y = c.GetAt(i).y;
		left = (totalSize + i - 6) % totalSize;
		right = (totalSize + i + 6) % totalSize;
		x1 = c.GetAt(left).x;
		y1 = c.GetAt(left).y;
		x2 = c.GetAt(right).x;
		y2 = c.GetAt(right).y;
		a = (y2 - y1);
		b = (x1 - x2);
		z = a * (x1)+ b * (y1);
		z *= -1;
		dist = fabs((a * x + b * y + z)) / (sqrt(a * a + b * b));
		lcs.Add(dist);
	}
}



void train(int fig_num, CArray<CPoint, CPoint>& c, CArray<double, double&>& lcs) {
	Mat img;
	if (fig_num == 1) {
		img = cv::imread("hex/1.jpg");
	}
	else if (fig_num == 2) {
		img = cv::imread("rect/1.jpg");
	}
	else {
		img = cv::imread("circle/1.jpg");
	}
	img=grayscale(img);
	segmentation(img);
	morp(img);
	contourTracing(img, c);
	lcs_extract(c, lcs);
}



void CRGBDlg::OnBnClickedLcs()
{
	lcs_array.RemoveAll();
	lcs_extract(contour_array, lcs_array);
}




void CRGBDlg::OnBnClickedMatching()
{
	double result[3];
	double smallest = (double)INF;
	int fig = 0;
	result[0] = DTW(lcs_array, hex_lcs);
	result[1] = DTW(lcs_array, rect_lcs);
	result[2] = DTW(lcs_array, circle_lcs);
	for (int i = 0; i < 3; i++) {
		if (smallest > result[i]) {
			smallest = result[i];
			fig = i + 1;
		}
	}
	switch (fig) {
	case 1:
		MessageBox(_T("이 도형은 육각형(hexagon) 입니다."), _T(""));
		break;
	case 2:
		MessageBox(_T("이 도형은 직사각형(rectangle) 입니다."), _T(""));
		break;
	case 3:
		MessageBox(_T("이 도형은 원(circle) 입니다."), _T(""));
		break;
	}

}	
