#!/bin/bash
# test_runner.sh - Запускает параметризованные тесты клиента

# Цвета для вывода
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo "=============================================="
echo "Запуск параметризованных тестов клиента"
echo "=============================================="

# Проверяем параметр протокола
if [ $# -ne 1 ]; then
    echo -e "${RED}Ошибка: Не указан протокол${NC}"
    echo "Использование: $0 <tcp|udp>"
    exit 1
fi

PROTOCOL="$1"
if [[ "$PROTOCOL" != "tcp" && "$PROTOCOL" != "udp" ]]; then
    echo -e "${RED}Ошибка: Неверный протокол${NC}"
    echo "Допустимые значения: tcp или udp"
    exit 1
fi

# Определяем порт на основе протокола
if [ "$PROTOCOL" == "tcp" ]; then
    PORT=23101
else
    PORT=23101
fi

# Определяем пути
BASE_DIR="$(pwd)"
TEST_DIR="${BASE_DIR}/unittests"
EXPECT_SCRIPT="${TEST_DIR}/test_read.expect"
CLIENT="${BASE_DIR}/client_sh"

# Проверки
if ! command -v expect &> /dev/null; then
    echo -e "${RED}Ошибка: expect не установлен${NC}"
    exit 1
fi

if [ ! -f "$CLIENT" ]; then
    echo -e "${RED}Ошибка: клиент не найден${NC}"
    exit 1
fi

if [ ! -x "$CLIENT" ]; then
    chmod +x "$CLIENT"
fi

if [ ! -x "$EXPECT_SCRIPT" ]; then
    chmod +x "$EXPECT_SCRIPT"
fi

# Массив тестов
# Формат: "test_name|graph_file|start|end|length|path"
TEST_CASES=(
    # Граф adj.txt (7 вершин, 0-4 не соединены с 5-6)
    "vertices_not_connected1|src/adj.txt|4|5|NO_PATH|VERTICES_NOT_CONNECTED"

    # Граф five.txt (5 вершин)
    # Ошибки
    "error_five_small_graph|src/five.txt|1|4|ERROR|ERROR_GRAPH_TOO_SMALL"
    "error_five_random|src/five.txt|0|3|ERROR|ERROR_GRAPH_TOO_SMALL"
    "error_five_start_end|src/five.txt|2|1|ERROR|ERROR_GRAPH_TOO_SMALL"

    # Верных нет

    # Граф six.txt (6 вершин)
    # Ошибки
    "error_six_start_vertex_not_found1|src/six.txt|22|1|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_six_start_vertex_not_found2|src/six.txt|6|3|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_six_vertex_not_found1|src/six.txt|0|11|ERROR|ERROR_VERTEX_NOT_FOUND"

    # Успешные тесты для six.txt
    "success_six_0_to_5|src/six.txt|0|5|3|0->1->3->5"
    "success_six_5_to_0|src/six.txt|5|0|3|5->3->1->0"
    "success_six_1_to_3|src/six.txt|1|3|1|1->3"
    "success_six_3_to_1|src/six.txt|3|1|1|3->1"
    "success_six_4_to_2|src/six.txt|4|2|1|4->2"
    "success_six_2_to_4|src/six.txt|2|4|1|2->4"
    "success_six_0_to_4|src/six.txt|0|4|2|0->2->4"
    "success_six_4_to_0|src/six.txt|4|0|2|4->2->0"

    # Граф thirteen.txt (13 вершин)
    # Ошибки
    "error_thirteen_start_vertex_not_found1|src/thirteen.txt|25|5|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_thirteen_start_vertex_not_found2|src/thirteen.txt|13|5|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_thirteen_vertex_not_found1|src/thirteen.txt|0|20|ERROR|ERROR_VERTEX_NOT_FOUND"

    # Успешные тесты для thirteen.txt
    "success_thirteen_0_to_12|src/thirteen.txt|0|12|4|0->1->4->9->12"
    "success_thirteen_12_to_0|src/thirteen.txt|12|0|4|12->9->4->1->0"
    "success_thirteen_2_to_11|src/thirteen.txt|2|11|2|2->6->11"
    "success_thirteen_11_to_2|src/thirteen.txt|11|2|2|11->6->2"
    "success_thirteen_1_to_8|src/thirteen.txt|1|8|2|1->3->8"
    "success_thirteen_8_to_1|src/thirteen.txt|8|1|2|8->3->1"
    "success_thirteen_0_to_10|src/thirteen.txt|0|10|3|0->2->5->10"
    "success_thirteen_10_to_0|src/thirteen.txt|10|0|3|10->5->2->0"

    # Граф nineteen.txt (19 вершин)
    # Ошибки
    "error_nineteen_start_vertex_not_found1|src/nineteen.txt|30|8|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_nineteen_start_vertex_not_found2|src/nineteen.txt|19|8|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_nineteen_vertex_not_found1|src/nineteen.txt|0|25|ERROR|ERROR_VERTEX_NOT_FOUND"

    # Успешные тесты для nineteen.txt
    "success_nineteen_0_to_17|src/nineteen.txt|0|17|4|0->1->4->11->17"
    "success_nineteen_17_to_0|src/nineteen.txt|17|0|4|17->11->4->1->0"
    "success_nineteen_3_to_16|src/nineteen.txt|3|16|2|3->9->16"
    "success_nineteen_16_to_3|src/nineteen.txt|16|3|2|16->9->3"
    "success_nineteen_2_to_14|src/nineteen.txt|2|14|2|2->7->14"
    "success_nineteen_14_to_2|src/nineteen.txt|14|2|2|14->7->2"
    "success_nineteen_1_to_12|src/nineteen.txt|1|12|2|1->5->12"
    "success_nineteen_12_to_1|src/nineteen.txt|12|1|2|12->5->1"
    "success_nineteen_0_to_18|src/nineteen.txt|0|18|4|0->1->5->12->18"
    "success_nineteen_18_to_0|src/nineteen.txt|18|0|4|18->12->5->1->0"

    # Граф twenty.txt (20 вершин)
    # Ошибки
    "error_twenty_start_vertex_not_found1|src/twenty.txt|30|9|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_twenty_start_vertex_not_found2|src/twenty.txt|20|9|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_twenty_vertex_not_found1|src/twenty.txt|0|25|ERROR|ERROR_VERTEX_NOT_FOUND"

    # Успешные тесты для twenty.txt
    "success_twenty_0_to_19|src/twenty.txt|0|19|3|0->3->9->19"
    "success_twenty_19_to_0|src/twenty.txt|19|0|3|19->9->3->0"
    "success_twenty_1_to_13|src/twenty.txt|1|13|2|1->5->13"
    "success_twenty_13_to_1|src/twenty.txt|13|1|2|13->5->1"
    "success_twenty_2_to_16|src/twenty.txt|2|16|2|2->7->16"
    "success_twenty_16_to_2|src/twenty.txt|16|2|2|16->7->2"
    "success_twenty_3_to_17|src/twenty.txt|3|17|2|3->8->17"
    "success_twenty_17_to_3|src/twenty.txt|17|3|2|17->8->3"
    "success_twenty_0_to_11|src/twenty.txt|0|11|3|0->1->4->11"
    "success_twenty_11_to_0|src/twenty.txt|11|0|3|11->4->1->0"

    # Граф twenty-one.txt (21 вершина) - слишком большой граф
    "error_twentyone_graph_too_large1|src/twenty-one.txt|21|3|ERROR|ERROR_GRAPH_TOO_LARGE"
    "error_twentyone_graph_too_large2|src/twenty-one.txt|30|10|ERROR|ERROR_GRAPH_TOO_LARGE"
    "error_twentyone_graph_too_large3|src/twenty-one.txt|0|30|ERROR|ERROR_GRAPH_TOO_LARGE"
    "error_twentyone_graph_too_large4|src/twenty-one.txt|21|10|ERROR|ERROR_GRAPH_TOO_LARGE"
)


echo "Тестовое окружение:"
echo "  Протокол:  $PROTOCOL"
echo "  Порт:      $PORT"
echo "  Клиент:    $CLIENT"
echo "  Тесты:     ${#TEST_CASES[@]}"
echo ""

PASSED=0
FAILED=0
SKIPPED=0

for test_case in "${TEST_CASES[@]}"; do
    # Разбираем тестовый случай
    IFS='|' read -r test_name graph_file start end length path <<< "$test_case"

    # Для тестов с ошибкой о маленьком графе не проверяем существование файла
    # (можем тестировать обработку несуществующих файлов)
    if [[ "$length" != "ERROR" ]] && [ ! -f "$BASE_DIR/$graph_file" ]; then
        echo -e "${YELLOW}Пропуск $test_name: файл $graph_file не найден${NC}"
        ((SKIPPED++))
        continue
    fi

    echo "=============================================="
    echo "Тест: $test_name"
    echo "  Граф: $graph_file"
    echo "  Путь: $start -> $end"
    echo "  Протокол: $PROTOCOL, Порт: $PORT"

    if [[ "$length" == "ERROR" ]]; then
        echo "  Ожидается: ОШИБКА - $path"
    elif [[ "$length" == "NO_PATH" ]]; then
        echo "  Ожидается: Вершины не соединены"
    else
        echo "  Ожидается: длина=$length, путь=$path"
    fi
    echo ""

    # Запускаем тест с передачей протокола
    if "$EXPECT_SCRIPT" "$CLIENT" "$graph_file" "$start" "$end" "$length" "$path" "$PROTOCOL" "$PORT"; then
        echo -e "${GREEN}✅ $test_name: ПРОЙДЕН${NC}"
        ((PASSED++))
    else
        echo -e "${RED}❌ $test_name: ПРОВАЛЕН${NC}"
        ((FAILED++))
    fi
    echo ""
done

echo "=============================================="
echo "ИТОГИ:"
echo -e "  Всего тестов: $((PASSED + FAILED + SKIPPED))"
echo -e "  Запущено:     $((PASSED + FAILED))"
echo -e "  ${GREEN}Пройдено:    $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "  ${RED}Провалено:  $FAILED${NC}"
fi
if [ $SKIPPED -gt 0 ]; then
    echo -e "  ${YELLOW}Пропущено:  $SKIPPED${NC}"
fi

if [ $FAILED -gt 0 ]; then
    exit 1
else
    echo -e "${GREEN}✅ Все запущенные тесты пройдены успешно!${NC}"
    exit 0
fi