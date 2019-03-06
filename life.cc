#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include "tbb/tbb.h"


using namespace::std;
using namespace::cv;
using namespace::boost;
using namespace::tbb;

int row,col;
int **txtData;
//int **cell_map;
vector<vector<int> > cell_map;
VideoWriter out;

struct arg_t {

    /** The input file */
    string in_file = "";

    /** The output file */
    string out_file = "";

    /** The frame rate */
    int fr;

    /** The number of pixels per cell */
    int ppc;

    /** The number of rounds */
    int num;

    /** The water mark */
    string mark = "";

    /** The out image */
    string out_img = "";

    /** Should we show the usage */
    bool usage = false;


};

int count_row(string filename){
    ifstream fileStream ;
    fileStream.open(filename,ios::in);
    string row_temp;
    if(fileStream.fail())//文件打开失败:返回0
    {
        cout<<"Read Error"<<endl;
        return 0;
    }
    else//文件存在
    {
        while(getline(fileStream,row_temp,'\n'))//读取一行
        {
            if (row_temp.size() > 0 )
                row++;
        }
    }
    fileStream.close();
    return row;
}

int count_col(string filename){
    ifstream fileStream ;
    fileStream.open(filename,ios::in);
    double col_temp = 0;
    //int col = 0;    // 列数计数器
    char c;            //当前位置的字符
    if(fileStream.fail())//文件打开失败:返回0
    {
        cout<<"Read Error"<<endl;
        return 0;
    }
    else//文件存在
    {
        c = fileStream.peek();
        while (('\n' != c) && (! fileStream.eof()))    // 指针指向的当前字符，仅观测，不移动指针位置
        {
            fileStream >> col_temp;
            col++;
            c = fileStream.peek();
        }
    }
    fileStream.close();
    return col;

}

void readin(int txt_row,int txt_col,string filename){
    txtData = new int*[txt_row];
    int i, j,m;
    FILE* fp = fopen((char*)filename.c_str(), "r"); //打开文件
    if (fp == NULL)
    {
        cout<<"Read Error"<<endl;
    }
    for (i = 0; i < txt_row; i++)
    {
        txtData[i] = new int[txt_col];
        for (j = 0; j < txt_col; j++)
        {
            fscanf(fp, "%d", &txtData[i][j]);/*每次读取一个数，fscanf函数遇到空格或者换行结束*/
        }
        fscanf(fp, "\n");
    }
    fclose(fp);
}

void parse_args(int argc, char **argv, arg_t &args) {
    long opt;
    while ((opt = getopt(argc, argv, "i:o:f:p:r:w:s:h")) != -1) {
        switch (opt) {
            case 'i':
                args.in_file = string(optarg);
                break;
            case 'o':
                args.out_file = string(optarg);
                break;
            case 'f':
                args.fr = stoi(optarg);
                break;
            case 'p':
                args.ppc = stoi(optarg);
                break;
            case 'r':
                args.num = stoi(optarg);
                break;
            case 'w':
                args.mark = string(optarg);
                break;
            case 's':
                args.out_img = string(optarg);
                break;
            case 'h':
                args.usage = true;
                break;
        }
    }
}

int main(int argc, char **argv){
    int i,j,m,n;
    arg_t args;
    parse_args(argc,argv,args);
    if (args.usage){
        cout << "-i: in-file" << endl;
        cout << "-o: out-file" << endl;
        cout << "-f: frame-rate" << endl;
        cout << "-p: number of pixels per cell" << endl;
        cout << "-r: number of rounds" << endl;
        cout << "-w: text-watermark" << endl;
        cout << "-s: out-image" << endl;
        cout << "-h: help" << endl;
        exit(0);
    }
    vector<string> results;
    boost::split(results,args.out_img,[](char c){ return c == ',';});

    count_col(args.in_file);
    count_row(args.in_file);
    //cell_map = new int*[row+2];
    //int **cell_temp = new int*[row+2];
    readin(row,col,args.in_file);
    cell_map = vector<vector<int> >(row+2,vector<int>(col+2));
    vector<vector<int> > cell_temp(row+2,vector<int>(col+2));
    bool label = out.open(args.out_file, CV_FOURCC('M', 'J', 'P', 'G'), args.fr,
                          Size((col+2)*args.ppc, (row+2)*args.ppc), false);


//    for (i = 0; i < row+2; i++)
//    {
//        cell_map[i] = new int[col+2];
//        cell_temp[i] = new int[col+2];
//    }

    for (i = 1; i < row+1; i++)
    {
        for (j = 1; j < col+1; j++)
        {
            cell_map[i][j]=*(*(txtData+i-1)+j-1);
            cell_temp[i][j]=*(*(txtData+i-1)+j-1);
        }
    }

    for(i=0;i<row+2;i++){
        cell_temp[i][0]=0;
        cell_temp[i][col+1]=0;
        cell_map[i][0]=0;
        cell_map[i][col+1]=0;

    }
    for(j=0;j<col+2;j++){
        cell_temp[0][j]=0;
        cell_temp[1][j]=0;
        cell_map[0][j]=0;
        cell_map[row+1][j]=0;
    }
//    for (i = 0; i < row+2; i++)
//    {
//        for (j = 0; j < col+2; j++)
//        {
//            cout<<cell_temp[i][j]<<" ";//输出
//        }
//        cout<<endl;
//    }
//    cout<<endl;
//    cout<<endl;
    for(m=0;m<args.num;m++){
        for (i = 1; i < row+1; i++)
        {
            for (j = 1; j < col+1; j++)
            {
                int dead=0;
                int live=0;
                if(cell_temp[i-1][j-1]==0){
                    dead+=1;
                }else{live+=1;}
                if(cell_temp[i-1][j]==0){
                    dead+=1;
                }else{live+=1;}
                if(cell_temp[i-1][j+1]==0){
                    dead+=1;
                }else{live+=1;}
                if(cell_temp[i][j-1]==0){
                    dead+=1;
                }else{live+=1;}
                if(cell_temp[i][j+1]==0){
                    dead+=1;
                }else{live+=1;}
                if(cell_temp[i+1][j-1]==0){
                    dead+=1;
                }else{live+=1;}
                if(cell_temp[i+1][j]==0){
                    dead+=1;
                }else{live+=1;}
                if(cell_temp[i+1][j+1]==0){
                    dead+=1;
                }else{live+=1;}
                if(live<2||live>3){
                    if(cell_temp[i][j]==1){
                        cell_map[i][j]=0;
                    }
                }else if(live==3){
                    if(cell_temp[i][j]==0){
                        cell_map[i][j]=1;
                    }
                }
            }
        }
        for (i = 1; i < row+1; i++)
        {
            for (j = 1; j < col+1; j++)
            {
                cell_temp[i][j]=cell_map[i][j];
            }
        }
        Mat M = Mat::zeros((row+2)*args.ppc,(col+2)*args.ppc,CV_8UC1);



        parallel_for(blocked_range<size_t>(0,row,1),
                     [&](const blocked_range<size_t> &r) {
                         for(size_t i = r.begin(); i != r.end() ; i++){
                             //for(i = 0; i<row+2 ; i++){
                             for(j = 0; j <col+2 ; j++){
                                 if(cell_map[i][j]==1){
                                     for(int k = i*args.ppc; k<(i+1)*args.ppc;k++){
                                         for(int l = j*args.ppc;l<(j+1)*args.ppc;l++){
                                             M.at<uchar>(k,l)=255;
                                         }
                                     }
                                 }
                             }
                             putText(M, args.mark, Point(50, 250), FONT_HERSHEY_DUPLEX, 1.8,
                                     cvScalar(100, 0, 0), 1, CV_AA);
                         }

                         out << M;

                     });
        if(results.size()!=0){
            for(n=0;n<results.size();n++){
                if(stoi(results[n])==m){
                    imwrite("frame"+results[n]+".png",M);
                }
            }
        }

    }
    out.release();


}