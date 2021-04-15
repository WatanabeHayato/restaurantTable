#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SalesDayNum 10 //営業日数

typedef struct //店舗情報
{
  char storeID[20];
  int seatTypeNum;
  char seatID[10][20];
  int seatNum[10];
  int seatMin[10];
  int seatMax[10];
  int saleTime[7][48];
} storeInfo;

typedef struct //残席数配列
{
  int remainSeatNum[SalesDayNum][48][10]; //[day][time][seatType]
}reserveState;

typedef struct
{
  char reservID[30];
  char userID[30];
  char storeID[20];
  char seatID[20];
  int useDay;
  int enterTime;
  int useTimeNum;
  int usePeopleNum;
  int dayOfWeek;
} requestInfo;

typedef struct
{
  char reservID[30];
  char userID[30];
  char storeID[20];
  char seatID[20];
  int useDay;
  int enterTime;
  int useTimeNum;
  int usePeopleNum;
  int useSeatNum;
} acceptInfo;

typedef struct //予約停止情報
{
  char storeID[20];
  char seatID[20];
  int stopDay;
  int stopTime;
  int stopTimeNum;
} stopInfo;


int calcUseSeatNum(int seatMin,int seatMax,int usePeopleNum){
  int minSeatNum,i,now=0;

  minSeatNum = usePeopleNum / seatMax;
  if(usePeopleNum % seatMax == 0){
    return minSeatNum;
  }
  minSeatNum++;

  int table[minSeatNum];
  
  for(i=0;i<minSeatNum-1;i++){
    table[i] = seatMax;
  }
  table[i] = usePeopleNum % seatMax;

  while(table[i] < seatMin){
    table[i]++;
    table[now]--;
    now++;
    if(now == i) now = 0;

    if(table[now] < seatMin){
      return -1;
    }
  }
  return i+1;

}

int main(void){
  storeInfo store[100]; //配列で店舗指定
  reserveState reserve[100]; //配列で店舗指定
  requestInfo req[100];
  acceptInfo accept[100];
  stopInfo stop[100];

  int storeNum,num=0;
  int i,j,k,l;
  char check;
  char query[20];
  char request[] = "REQUEST";
  char nextDay[] = "NEXT_DAY";
  char show[] = "SHOW";
  char cancel[] = "CANCEL";
  char STOP[] = "STOP";
  char list[] = "LIST";
  char remove[] = "REMOVE";
  int focusStore,focusSeat,useTimeCheck,closed;
  int today = 1;
  int DayOfWeek;
  int useSeatNum = 0;
  int acceptNum = 0;
  int overlapped;
  int storeCheck,seatCheck;
  int enoughSeat;
  int ploblemPeopleNum;
  int minSeatNum;
  char space[] = " ";

  //showで使う変数
  char showStoreID[20];
  int showDay,reserveCheck;

  //removeで使う変数
  char removeStoreID[20];
  char cannnotCancel[100][20];
  int cannotCancelNum = 0;

  //cancelで使う変数
  char cancelUserID[20];
  char cancelReserveID[20];
  char cancelStoreID[20];
  char cancelSeatID[20];
  int cancelUseDay,cancelEnterTime,cancelUseTimeNum,cancelPeopleNum,cancelSeatNum,pastReserve,cancelNum;

  //stopで使う変数
  char stopStoreID[20];
  int stopDay,stopTime,stopTimeNum;
  char stopSeatID[20];
  char stopSeatCheck;
  int stopNum = 0;
  
  for(i=0;i<100;i++){
    for(j=0;j<SalesDayNum;j++){
      for(k=0;k<48;k++){
        for(l=0;l<10;l++){
          reserve[i].remainSeatNum[j][k][l] = -1; //残隻数配列の初期化
        }
      }
    }
  }
  //printf("%d\n",reserve[1].remainSeatNum[1][1][1]);

  scanf("%d",&storeNum);
  for(i=0;i<storeNum;i++){
    scanf("%s %d",store[i].storeID,&store[i].seatTypeNum);
    for(j=0;j<store[i].seatTypeNum;j++){
      scanf("%s %d %d %d",store[i].seatID[j],&store[i].seatNum[j],&store[i].seatMin[j],&store[i].seatMax[j]);
    }
    for(j=0;j<7;j++){
      k=0;
      do{
        scanf("%d",&store[i].saleTime[j][k]);
        scanf("%c",&check);
        //printf("%d\n",store[i].saleTime[j][k]);
        k++;
      }while(check != '\n');
    }
  }

  while(scanf("%s",query) != EOF){
    focusStore = -1;
    focusSeat = -1;
    useTimeCheck = -1;
    closed = 1;
    useSeatNum = 0;
    overlapped = 0;
    storeCheck = -1;
    seatCheck = -1;
    enoughSeat = 1;
    ploblemPeopleNum = 1;
    minSeatNum = 0;
    reserveCheck = -1;
    pastReserve = 1;

    DayOfWeek = today % 7;
    if(DayOfWeek == 0) DayOfWeek = 7;

    if(strcmp(query,request) == 0){
      scanf("%s %s %s %s %d %d %d %d",req[num].reservID,req[num].userID,req[num].storeID,req[num].seatID,&req[num].useDay,&req[num].enterTime,&req[num].useTimeNum,&req[num].usePeopleNum);
      //printf("%s\n",req[num].storeID);

      for(i=0;i<storeNum;i++){ //リクエストのstoreIDが存在するかチェック
        if(strcmp(store[i].storeID,req[num].storeID) == 0){
          focusStore = i;
          storeCheck = 1;
          break;
        }
      }

      for(j=0;j<store[focusStore].seatTypeNum;j++){ //リクエストのseatIDが存在するかチェック
        if(strcmp(store[focusStore].seatID[j],req[num].seatID) == 0){
          focusSeat = j;
          seatCheck = 1;
          break;
        }
      }

      //リクエストの時間に開いているかチェック
      req[num].dayOfWeek = req[num].useDay % 7;
      if(req[num].dayOfWeek == 0) req[num].dayOfWeek = 7;
      for(i=0;i<req[num].useTimeNum;i++){
        j=0;
        useTimeCheck = -1;
        while(store[focusStore].saleTime[req[num].dayOfWeek][j] != 0){
          if(req[num].enterTime + i == store[focusStore].saleTime[req[num].dayOfWeek][j]) useTimeCheck = 1;
          j++;
        }
        if(useTimeCheck != 1){
          closed = -1;
          break;
        }
      }

      //printf("debug:%d %d %d\n",store[focusStore].seatMin[focusSeat],store[focusStore].seatMax[focusSeat],req[num].usePeopleNum);

      //使用席数を計算
      if(storeCheck == 1 && seatCheck == 1){
        useSeatNum = calcUseSeatNum(store[focusStore].seatMin[focusSeat],store[focusStore].seatMax[focusSeat],req[num].usePeopleNum);
      }

      //予約が重複していないかチェック
      for(i=0;i<acceptNum;i++){
        if(strcmp(req[num].userID,accept[i].userID) == 0){
          if(req[num].useDay == accept[i].useDay){
            for(j=accept[i].enterTime;j< accept[i].enterTime + accept[i].useTimeNum;j++){
              for(k=req[num].enterTime;k< req[num].enterTime + req[num].useTimeNum;k++){
                if(j == k) overlapped = -1;
              }
            }
          }
        }
      }

      //席が空いているかチェック
      if(reserve[focusStore].remainSeatNum[0][0][focusSeat] == -1){
       for(j=0;j<SalesDayNum;j++){
         for(k=0;k<48;k++){
            reserve[focusStore].remainSeatNum[j][k][focusSeat] = store[focusStore].seatNum[focusSeat]; //残席数配列の初期化
         }
       }
      }

      //stopされていないかチェック
      for(i=0;i<stopNum;i++){
        if(strcmp(stop[i].storeID,store[focusStore].storeID) == 0){
          if(stop[i].stopDay == req[num].useDay){
            for(k=stop[i].stopTime;k<stop[i].stopTime + stop[i].stopTimeNum;k++){
              for(j=req[num].enterTime;j<req[num].enterTime + req[num].useTimeNum;j++){
                if(k == j){
                  if(strcmp(stop[i].seatID,space) == 0) closed = -1;
                  else if(strcmp(stop[i].seatID,req[num].seatID) == 0) closed = -1;
                }
              }
            }
          }
        }
      }

      if(useSeatNum != -1){
       for(i=req[num].enterTime;i< req[num].enterTime + req[num].useTimeNum;i++){
         if(reserve[focusStore].remainSeatNum[req[num].useDay][i][focusSeat] < useSeatNum){
           enoughSeat = -1;
           break;
         }
        }
      }

      //too many or too few peopleの判定
      if(req[num].usePeopleNum < store[focusStore].seatMin[focusSeat]){
        ploblemPeopleNum = -1;
      }else{
        minSeatNum = reserve[focusStore].remainSeatNum[req[num].useDay][req[num].enterTime][focusSeat];
        for(i=req[num].enterTime;i< req[num].enterTime + req[num].useTimeNum;i++){
         if(reserve[focusStore].remainSeatNum[req[num].useDay][i][focusSeat] < minSeatNum){
           minSeatNum = reserve[focusStore].remainSeatNum[req[num].useDay][i][focusSeat];
         }
        }
        if(minSeatNum * store[focusStore].seatMax[focusSeat] < req[num].usePeopleNum) ploblemPeopleNum = -1;
      }

      if(storeCheck == -1) printf("Error: No such reataurant\n");
      else if(seatCheck == -1) printf("Error: No such table\n");
      else if(req[num].useDay < today) printf("Error: Past date is specified\n");
      else if(closed == -1) printf("Error: Closed\n");
      else if(overlapped == -1) printf("Error: Overlapped reservation\n");
      else if(ploblemPeopleNum == -1) printf(("Error: Too many or too few people\n"));
      else if(useSeatNum == -1) printf("Error: Unable to divide into groups\n");
      else{
        printf("REQUEST-ACCEPTED %s %s %s %s %d %d %d %d %d\n",req[num].reservID,req[num].userID,req[num].storeID,req[num].seatID,req[num].useDay,req[num].enterTime,req[num].useTimeNum,req[num].usePeopleNum,useSeatNum);
        strcpy(accept[acceptNum].reservID,req[num].reservID);
        strcpy(accept[acceptNum].userID,req[num].userID);
        strcpy(accept[acceptNum].storeID,req[num].storeID);
        strcpy(accept[acceptNum].seatID,req[num].seatID);
        accept[acceptNum].useDay = req[num].useDay;
        accept[acceptNum].enterTime = req[num].enterTime;
        accept[acceptNum].useTimeNum = req[num].useTimeNum;
        accept[acceptNum].usePeopleNum = req[num].usePeopleNum;
        accept[acceptNum].useSeatNum = useSeatNum;

        for(i=accept[acceptNum].enterTime;i< accept[acceptNum].enterTime + accept[acceptNum].useTimeNum;i++){
          reserve[focusStore].remainSeatNum[accept[acceptNum].useDay][i][focusSeat] -= accept[acceptNum].useSeatNum;
        }
        acceptNum++;
      }
    }

    if(strcmp(query,nextDay) == 0){//ソート未実装
      today++;
      for(i=0;i<acceptNum;i++){
        if(accept[i].useDay == today+1){
          printf("REMINDER %s %s %s %s %d %d %d %d\n",accept[i].userID,accept[i].reservID,accept[i].storeID,accept[i].seatID,accept[i].useDay,accept[i].enterTime,accept[i].useTimeNum,accept[i].usePeopleNum);
        }
      }
    }

    if(strcmp(query,show) == 0){
      scanf("%s %d",showStoreID,&showDay);

      //storeIDが存在するかチェック
      for(i=0;i<storeNum;i++){ 
        if(strcmp(store[i].storeID,showStoreID) == 0){
          storeCheck = 1;
          focusStore = i;
          break;
        }
      }

      //指定された日付に予約があるかチェック
      for(i=0;i<acceptNum;i++){
        if(accept[i].useDay == showDay) reserveCheck = 1;
      }

      if(storeCheck == -1) printf("Error: No such restaurant\n");
      else if(reserveCheck == -1) printf("There are no reservations\n");
      else{//ソート未実装
        for(i=0;i<acceptNum;i++){
          if(accept[i].useDay == showDay && strcmp(accept[i].storeID,store[focusStore].storeID) == 0){
            printf("SHOW-RESULT %s %d %d %d %d %d %s %s\n",accept[i].reservID,accept[i].useDay,accept[i].enterTime,accept[i].useTimeNum,accept[i].usePeopleNum,accept[i].useSeatNum,accept[i].seatID,accept[i].userID);
          }
        }
      }
    }

    if(strcmp(query,cancel) == 0){
      scanf("%s %s",cancelUserID,cancelReserveID);

      //予約があるかチェック
      for(i=0;i<acceptNum;i++){
        if(strcmp(cancelReserveID,accept[i].reservID) == 0){
          if(strcmp(cancelUserID,accept[i].userID) == 0){
            reserveCheck = 1;
            strcpy(cancelStoreID,accept[i].storeID);
            strcpy(cancelSeatID,accept[i].seatID);
            cancelEnterTime = accept[i].enterTime;
            cancelUseTimeNum = accept[i].useTimeNum;
            cancelPeopleNum = accept[i].usePeopleNum;
            cancelSeatNum = accept[i].useSeatNum;
            cancelUseDay = accept[i].useDay;
            cancelNum = i;
          }
        }
      }
      
      //掲載中止にされていないかチェック
      for(i=0;i<storeNum;i++){
        if(strcmp(store[i].storeID,cancelStoreID) == 0){
          storeCheck = 1;
          focusStore = i;
        }
      }

      for(j=0;j<store[focusStore].seatTypeNum;j++){ 
        if(strcmp(store[focusStore].seatID[j],cancelSeatID) == 0){
          focusSeat = j;
          break;
        }
      }

      //現在の日付が予約日より前かチェック
      if(today > cancelUseDay) pastReserve = -1;

      if(reserveCheck == -1) printf("Error: Not found\n");
      else if(storeCheck == -1) printf("Error: No such restaurant\n");
      else if(pastReserve == -1) printf("Error: Past reservation\n");
      else{
        printf("CANCEL-ACCEPTED %s\n",cancelReserveID);
        strcpy(accept[cancelNum].reservID,space);
        //席の数を足す
        for(i=cancelEnterTime;i<cancelEnterTime + cancelUseTimeNum;i++){
          reserve[focusStore].remainSeatNum[cancelUseDay][i][focusSeat] += cancelSeatNum;
        }
      }

    }

    if(strcmp(query,STOP) == 0){
      scanf("%s %d %d %d",stopStoreID,&stopDay,&stopTime,&stopTimeNum);
      scanf("%c",&stopSeatCheck);

      //storeIDが存在するかチェック
      for(i=0;i<storeNum;i++){ 
        if(strcmp(store[i].storeID,stopStoreID) == 0){
          storeCheck = 1;
          focusStore = i;
          break;
        }
      }

      if(stopSeatCheck == ' '){
        scanf("%s",stopSeatID);

        //席種が存在するかチェック
        for(i=0;i<store[focusStore].seatNum;i++){
          if(strcmp(store[focusStore].seatID[i],stopSeatID) == 0){
           seatCheck = 1;
           break;
          }
        }
      }

      //予約停止日が現在の日付より前かチェック
      if(stopDay < today){
        pastReserve = -1;
      }

      if(storeCheck == -1) printf("Error: No such restaurant\n");
      else if(pastReserve == -1) printf("Error: Specify a date or after today\n");
      else if(seatCheck == -1) printf("Error: No such table\n");
      else{
        strcpy(stop[stopNum].storeID,stopStoreID);
        if(stopSeatCheck == ' '){
          strcpy(stop[stopNum].seatID,stopSeatID);
        }else{
          strcpy(stop[stopNum].seatID,space);
        }
        stop[stopNum].stopDay = stopDay;
        stop[stopNum].stopTime = stopTime;
        stop[stopNum].stopTimeNum = stopTimeNum;
        printf("STOP-ACCEPTED %s\n",stopStoreID);
        stopNum++;
      }

    }

    if(strcmp(query,list) == 0){
      storeNum++;
      scanf("%s %d",store[storeNum].storeID,&store[storeNum].seatTypeNum);
      for(j=0;j<store[storeNum].seatTypeNum;j++){
        scanf("%s %d %d %d",store[storeNum].seatID[j],&store[storeNum].seatNum[j],&store[storeNum].seatMin[j],&store[storeNum].seatMax[j]);
      }
      for(j=0;j<7;j++){
       k=0;
        do{
          scanf("%d",&store[storeNum].saleTime[j][k]);
          scanf("%c",&check);
          //printf("%d\n",store[i].saleTime[j][k]);
          k++;
        }while(check != '\n');
      }
      printf("LIST-ACCEPTED %s\n",store[storeNum].storeID);
    }

    //キャンセルできないようにする処理はcancelで判断できるようにする
    if(strcmp(query,remove) == 0){
      scanf("%s",removeStoreID);
      for(i=0;i<storeNum;i++){
        if(strcmp(store[i].storeID,removeStoreID) == 0) storeCheck = 1;
      }
      if(storeCheck == -1) printf("Error: No such restaurant\n");
      else{
        for(i=0;i<storeNum;i++){
          if(strcmp(store[i].storeID,removeStoreID) == 0){
            strcpy(cannnotCancel[cannotCancelNum],store[i].storeID);
            strcpy(store[i].storeID,space);
            printf("REMOVE-ACCEPTED %s\n",removeStoreID);
            cannotCancelNum++;
          }
        }
      }
    }
    num++;
  }

  return 0;
}