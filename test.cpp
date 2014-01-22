
#include <stdio.h>
#include <stdlib.h>
#include "base_type.h"
#include <time.h>

TypeHintCostB optimize(OptimizeTask task);
int main(){
 FILE* file=fopen("optimizeTask.dat","rb");
 OptimizeTask task;
 fread(&task,sizeof(OptimizeTask),1,file);
 task.capCost=1000000;
 for(int t=0;t<task.typeSize;t++){
  printf("test[t]: %llx\n",task.typeTestMask[t]);
 }
 printf("memberSize: %d\n",task.memberSize);
 TypeHintCostB f;
 size_t time=clock();
 for(int t=0;t<1000;t++){
  f=optimize(task);
 }
 time = clock() - time;
 printf ("It took me %d clicks (%f seconds).\n",time,((float)time)/CLOCKS_PER_SEC);
 size_t t2=clock();
 printf("starting!\n");
 for(int t=0;t<task.memberSize;t++){
  printf("%d ",f.member[t]);
 }
 printf("\n");
}
TypeHintCostB optimize(OptimizeTask task){
 //used as final return
 TypeHintCostB f;
 //index of type with minCost for given member
 Byte memberMinIndex[MAX_MEMBER];

 //used for recursion

 //bundle[n+1]=typeIndex for bundle n;
 Byte bundle[MAX_BUNDLE+2];
 //bundleLength[n] is the remaining totalLength after having first n bundles
 Double bundleLength[MAX_BUNDLE+1];
 //bundleCost[n] is the totalCost after having first n bundles
 Dollar bundleCost[MAX_BUNDLE+1];
 //alternative to memberBundleIndex
 TestMask bundleRemain[MAX_BUNDLE+1];
 Dollar bundleMinCost[MAX_BUNDLE+1];

 f.cost=task.capCost;

 //update bundleLength[0]
 bundleLength[0]=0;
 for(int t=0;t<task.memberSize;++t){
  bundleLength[0]+=task.length[t];
 }

 //update memberMinIndex, bundleMinCost[0]
 bundleMinCost[0]=0;
 bundleRemain[0]=(TestMask)(1L<<task.memberSize)-1;
 for(int t=0;t<task.typeSize;++t){
  TestMask valid=bundleRemain[0]&task.typeTestMask[t];
  bundleRemain[0]^=valid;
  int tmp=0;
  while(valid!=0){
   if(valid&1){
    memberMinIndex[tmp]=t;
    bundleMinCost[0]+=task.length[tmp]*task.cost[t];
   }
   valid>>=1;
   ++tmp;
  }
 }

 bundleRemain[0]=(TestMask)(1L<<task.memberSize)-1;
 bundleCost[0]=0;
 bundle[0]=0;
 bundle[1]=0;
 

 int level=0;
 while(level>-1){
  //TODO don't check when just decreased level?
 
  //finished?
  if(bundleRemain[level]==0){
   //new best?
   if(bundleCost[level] < f.cost){
    //copy to f
    f.cost=bundleCost[level];
    for(int t=0;t<level;++t){
     f.bundle[t]=task.index[bundle[t+1]];
    }
    for(int t=0;t<task.memberSize;++t){
     TestMask tmpMask=((TestMask)(1))<<t;
     Byte tmp=level-1;
     while(true){
      if(task.typeTestMask[bundle[tmp+1]]&bundleRemain[tmp]&(tmpMask)){
       f.member[t]=tmp;
       break;
      }
      --tmp;
     }
    }
   }
   level-=2;
   continue;
  }
  //stop when cost too large
  if(bundleCost[level] + bundleMinCost[level] > f.cost){
   --level;
   continue;
  }
  //stop when bundle total length too small
  if(level>0 && (bundleLength[level-1] - bundleLength[level] <task.minLength)){
   --level;
   continue;
  }
  //too many bundle
  if(level>=MAX_BUNDLE){
   --level;
   continue;
  }

  //tested all types
  if(bundle[level+1]==task.typeSize-1){
   --level;
   continue;
  }
  
  ++level;
  ++bundle[level];
  TestMask valid=bundleRemain[level-1]&task.typeTestMask[bundle[level]];
  if(valid==0){
    --level;
    continue;
  }
  bundleLength[level]=bundleLength[level-1];
  bundleCost[level]=bundleCost[level-1]+task.bundleCost;
  bundleMinCost[level]=bundleMinCost[level-1];
  bundleRemain[level]=bundleRemain[level-1]^valid;
  if(level<MAX_BUNDLE){
   bundle[level+1]=bundle[level]+1;
  }
  //TODO optimize performance, take advantage of bitset.
  while(valid!=0){
   int t=FFSLL(valid)-1;
   bundleLength[level] -= task.length[t];
   bundleCost[level] +=task.cost[bundle[level]]*task.length[t];
   bundleMinCost[level]-=task.cost[memberMinIndex[t]]*task.length[t];
   valid^=1L<<t;
  }
 }
 return f;
}