#!/bin/bash

PROGRAM="./testDIOL"  # 실행 파일 경로를 지정하세요

size=100000000
datas=("data_c100000000_d0.10" "data_c100000000_d0.20" "data_c100000000_d0.30")

if [ "$#" -ne 1 ]; then
  echo "Usage: $0 <value>"
  echo "Allowed values: A B C D E"
  exit 1
fi

VALUE=$1

# data와 files 배열 설정
case $VALUE in
  A)
    files=("workloadA_data_c100000000_d0.10" "workloadA_data_c100000000_d0.20" "workloadA_data_c100000000_d0.30")
    ;;
  B)
    files=("workloadB_data_c100000000_d0.10" "workloadB_data_c100000000_d0.20" "workloadB_data_c100000000_d0.30")
    ;;
  C)
    files=("workloadC_data_c100000000_d0.10" "workloadC_data_c100000000_d0.20" "workloadC_data_c100000000_d0.30")
    ;;
  D)
    files=("workloadD_data_c100000000_d0.10" "workloadD_data_c100000000_d0.20" "workloadD_data_c100000000_d0.30")
    ;;
  E)
    files=("workloadE_data_c100000000_d0.05" "workloadE_data_c100000000_d0.10" "workloadE_data_c100000000_d0.20" "workloadE_data_c100000000_d0.30")
    ;;
  *)
    echo "Invalid value: $VALUE"
    echo "Allowed values: A, B, C, D, E"
    exit 1
    ;;
esac

# datas와 files에서 하나씩 선택하여 실행
for i in "${!datas[@]}"; do
    data=${datas[$i]}
    file=${files[$i]}
    echo "[ DIOL ] Running with data=${data} and workload=${file}"
    $PROGRAM $size $data $file
done
