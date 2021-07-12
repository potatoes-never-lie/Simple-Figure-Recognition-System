# Simple-Figure-Recognition-System
This project provides recognition between rectangle, hexagon and circle. (2021-1 Computer Vision Project)


이 프로그램은 육각형(hexagon)/직사각형(rectangle)/원(circle) 영상 이미지를 학습시키고, 입력 영상에 대해 위 3개의 도형 중 어떤 도형인지 판단하는 프로그램이다.

각 도형의 LCS(e Longest Common Subsequence) 를 추출하여, DTW(Dynamic Time Warping)를 수행하여 가장 작은 dissimilarity를 가진 도형에 대해 입력 영상에 맞는 도형을 매칭시킨다.
