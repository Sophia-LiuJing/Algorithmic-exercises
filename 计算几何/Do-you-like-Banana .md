# Do you like Banana 

## 题目描述

Two endpoints of two line segments on a plane are given to determine whether the two segments are intersected (there is a common point or there is a partial coincidence that intersects). If intersected, output "Yes", otherwise output "No".

## 代码

```c++
#include <bits/stdc++.h>
using namespace std;
struct point//一个结构体用来存储一个点
{
 double x,y;//分别存储点的两个坐标
 point(double x=0,double y=0):x(x),y(y){}//构造函数
};
typedef point Vector;//向量在代码中其实与点差不多，因此可以直接typedef一下
inline Vector operator + (Vector A,Vector B) {return Vector(A.x+B.x,A.y+B.y);}//向量+向量=向量
inline Vector operator - (point  A,point  B) {return Vector(A.x-B.x,A.y-B.y);}//点-点=向量
inline Vector operator * (Vector A,double x) {return Vector(A.x*x,A.y*x);}//向量*一个数=向量
inline Vector operator / (Vector A,double x) {return Vector(A.x/x,A.y/x);}//向量/一个数=向量
inline double cross(Vector A,Vector B) {return A.x*B.y-A.y*B.x;}//叉积
//判断两个线段是否交叉
bool intersection(point a,point b,point c,point d)
{
    //快速排斥实验
    if(max(c.x,d.x)<min(a.x,b.x)||max(a.x,b.x)<min(c.x,d.x)||max(c.y,d.y)<min(a.y,b.y)||max(a.y,b.y)<min(c.y,d.y)){
        return false;
    }
    //跨立实验
    if(cross(a-d,c-d)*cross(b-d,c-d)>0||cross(d-b,a-b)*cross(c-b,a-b)>0){
        return  false;
    }
    return true;
}

int main(){
    int n;
    scanf("%d",&n);
    for(int i=0;i<n;i++){
        point a,b,c,d;
        scanf("%lf %lf %lf %lf %lf %lf %lf %lf",&a.x,&a.y,&b.x,&b.y,&c.x,&c.y,&d.x,&d.y);
        if(intersection(a,b,c,d)) printf("Yes\n");
        else printf("No\n");
        
    }
    return 0;
}
```