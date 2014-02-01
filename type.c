#include "type.h"
#include <string.h>
#include <stdlib.h>
#include <math.h>

int compareTypeCost (const void *a, const void *b){
    const Double *da = &((const TypeB*) a)->cost;
    const Double *db = &((const TypeB*) b)->cost;
                                
    return (*da > *db) - (*da < *db);
}

int setupTypes(TypeB* types,const char* buf){
    for(int t=0;t<MAX_TYPE;t++){
        int i1,i2,i3;
        Double E,Fy,Density,Area,Moment,CostVol;
        char name[6];
        int n=sscanf(buf,"%d %d %d %5s %lf %lf %lf %lf %lf %lf\n",&i1,&i2,&i3,name,&E,&Fy,&Density,&Area,&Moment,&CostVol);
        while(*(buf++)!='\n'){}
        if(n!=10){
            return 1;
        }
        types[t].index=(Byte)(MATERIAL_SHIFT*i1+SHAPE_SHIFT*i2+SIZE_SHIFT*i3);
        strcpy(types[t].name,name);
        types[t].weight=DEAD_LOAD_FACTOR*Density*Area*GRAVITY/2000;
        types[t].AE=Area*E;
        types[t].cost=CostVol*Area*Density*Area*GRAVITY*2;
        types[t].tensionStrength=TENSION_RESISTANCE_FACTOR*Fy*Area;
        types[t].inverseRadiusOfGyration=sqrt(Area/Moment);
        types[t].FyArea_d_CEMoment=Fy*Area/(PI2*E*Moment);
        types[t].FyArea=Fy*Area;
    }
    qsort(types,MAX_TYPE,sizeof(TypeB),compareTypeCost);
    for(int t=0;t<MAX_TYPE;++t){
        types[t].index2=t;
    }
    
    return 0;
}

bool ifPassType(const TypeB* type,Double compression,Double tension,Double length,Double slenderness){
  return (tension <= type->tensionStrength)&&
         (slenderness>length*type->inverseRadiusOfGyration)&&
         (compression <= getCompressionStrength(type,length));
}
Double getCompressionStrength(const TypeB* type,Double length){
  Double lambda=length*length*type->FyArea_d_CEMoment;
  return lambda<=2.25?
           COMPRESSION_RESISTANCE_FACTOR*pow(0.66,lambda)*type->FyArea:
           0.792*type->FyArea/lambda;
}

