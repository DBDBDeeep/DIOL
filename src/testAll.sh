
# 실행할 쉘 파일 경로
SCRIPT_TO_RUN="./test_workload.sh"

# A부터 G까지의 값 목록
values=("A" "B" "D")

# A부터 G까지 순차적으로 실행
for VALUE in "${values[@]}"; do
  echo "Running $SCRIPT_TO_RUN with value $VALUE"
  bash "$SCRIPT_TO_RUN" "$VALUE"
done
