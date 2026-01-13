#!/bin/bash
# input_tests.sh - Запускает параметризованные тесты клиента с ручным вводом графа

# Цвета для вывода
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

echo "=============================================="
echo "Запуск параметризованных тестов клиента (ручной ввод)"
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
EXPECT_SCRIPT="${TEST_DIR}/test_input.expect"
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

if [ ! -f "$EXPECT_SCRIPT" ]; then
    echo -e "${RED}Ошибка: скрипт тестирования не найден${NC}"
    exit 1
fi

if [ ! -x "$EXPECT_SCRIPT" ]; then
    chmod +x "$EXPECT_SCRIPT"
fi

# Массив тестов
# Формат: "test_name|vertices|edges|start|end|length|path"
# edges формат: вершина:сосед1,сосед2,...;вершина:сосед1,...
TEST_CASES=(
    # Граф из 5 вершин (слишком маленький)
    "error_input_5_vertices|5|0:1,2;1:0,3;2:0,4;3:1,5;4:2|1|4|ERROR|ERROR_GRAPH_TOO_SMALL"

    # Граф из 6 вершин (src/six.txt)
    "success_input_six_1_to_4|6|0:1,2;1:0,3;2:0,4;3:1,5;4:2;5:3|1|4|3|1->0->2->4"
    "success_input_six_0_to_5|6|0:1,2;1:0,3;2:0,4;3:1,5;4:2;5:3|0|5|3|0->1->3->5"
    "success_input_six_2_to_3|6|0:1,2;1:0,3;2:0,4;3:1,5;4:2;5:3|2|3|3|2->0->1->3"
    "success_input_six_0_to_4|6|0:1,2;1:0,3;2:0,4;3:1,5;4:2;5:3|0|4|2|0->2->4"

    # Граф из 13 вершин (src/thirteen.txt)
    "success_input_thirteen_0_to_12|13|0:1,2;1:0,3,4;2:0,5,6;3:1,7,8;4:1,9;5:2,10;6:2,11;7:3;8:3;9:4,12;10:5;11:6;12:9|0|12|4|0->1->4->9->12"
    "success_input_thirteen_2_to_11|13|0:1,2;1:0,3,4;2:0,5,6;3:1,7,8;4:1,9;5:2,10;6:2,11;7:3;8:3;9:4,12;10:5;11:6;12:9|2|11|2|2->6->11"
    "success_input_thirteen_0_to_10|13|0:1,2;1:0,3,4;2:0,5,6;3:1,7,8;4:1,9;5:2,10;6:2,11;7:3;8:3;9:4,12;10:5;11:6;12:9|0|10|3|0->2->5->10"

    # Граф из 19 вершин (src/nineteen.txt)
    "success_input_nineteen_0_to_17|19|0:1,2,3;1:0,4,5;2:0,6,7;3:0,8,9;4:1,10,11;5:1,12;6:2,13;7:2,14;8:3,15;9:3,16;10:4;11:4,17;12:5,18;13:6;14:7;15:8;16:9;17:11;18:12|0|17|4|0->1->4->11->17"
    "success_input_nineteen_3_to_16|19|0:1,2,3;1:0,4,5;2:0,6,7;3:0,8,9;4:1,10,11;5:1,12;6:2,13;7:2,14;8:3,15;9:3,16;10:4;11:4,17;12:5,18;13:6;14:7;15:8;16:9;17:11;18:12|3|16|2|3->9->16"
    "success_input_nineteen_0_to_18|19|0:1,2,3;1:0,4,5;2:0,6,7;3:0,8,9;4:1,10,11;5:1,12;6:2,13;7:2,14;8:3,15;9:3,16;10:4;11:4,17;12:5,18;13:6;14:7;15:8;16:9;17:11;18:12|0|18|4|0->1->5->12->18"

    # Граф из 20 вершин (src/twenty.txt)
    "success_input_twenty_0_to_19|20|0:1,2,3;1:0,4,5;2:0,6,7;3:0,8,9;4:1,10,11;5:1,12,13;6:2,14;7:2,15,16;8:3,17;9:3,18,19;10:4;11:4;12:5;13:5;14:6;15:7;16:7;17:8;18:9;19:9|0|19|3|0->3->9->19"
    "success_input_twenty_1_to_13|20|0:1,2,3;1:0,4,5;2:0,6,7;3:0,8,9;4:1,10,11;5:1,12,13;6:2,14;7:2,15,16;8:3,17;9:3,18,19;10:4;11:4;12:5;13:5;14:6;15:7;16:7;17:8;18:9;19:9|1|13|2|1->5->13"
    "success_input_twenty_0_to_11|20|0:1,2,3;1:0,4,5;2:0,6,7;3:0,8,9;4:1,10,11;5:1,12,13;6:2,14;7:2,15,16;8:3,17;9:3,18,19;10:4;11:4;12:5;13:5;14:6;15:7;16:7;17:8;18:9;19:9|0|11|3|0->1->4->11"

    # Граф из 21 вершин (слишком большой)
    "error_input_21_vertices|21|0:1,2,3;1:0,4,5;2:0,6,7;3:0,8,9;4:1,10,11;5:1,12,13;6:2,14,15;7:2,16,17;8:3,18,19;9:3,20;10:4;11:4;12:5;13:5;14:6;15:6;16:7;17:7;18:8;19:8;20:9|0|20|ERROR|ERROR_GRAPH_TOO_LARGE"

    # Ошибки: вершины не найдены
    "error_input_start_not_found|6|0:1,2;1:0,3;2:0,4;3:1,5;4:2;5:3|10|2|ERROR|ERROR_START_VERTEX_NOT_FOUND"
    "error_input_end_not_found|6|0:1,2;1:0,3;2:0,4;3:1,5;4:2;5:3|2|10|ERROR|ERROR_VERTEX_NOT_FOUND"

    # Тест с несвязными вершинами (граф adj.txt - 0-4 не соединены с 5-6)
    "input_vertices_not_connected|7|0:1,2,3,4;1:0,2,3,4;2:0,1,3,4;3:0,1,2,4;4:0,1,2,3;5:6;6:5|4|5|NO_PATH|VERTICES_NOT_CONNECTED"

    # Различные вариации несвязных графов
    "input_disconnected_graph1|8|0:1,2;1:0,2;2:0,1;3:4;4:3;5:6,7;6:5,7;7:5,6|0|3|NO_PATH|VERTICES_NOT_CONNECTED"
    "input_disconnected_graph2|8|0:1;1:0;2:3;3:2;4:5;5:4;6:7;7:6|0|7|NO_PATH|VERTICES_NOT_CONNECTED"

    # Граф с петлями (самосвязями недопустимы)
    "input_self_loop_test|6|0:0,1,2;1:0,3;2:0,4;3:1,5;4:2;5:3|0|5|3|0->1->3->5"

    # Граф с кратными ребрами (будут проигнорированы или обработаны)
    "input_multiple_edges_test|6|0:1,1,2,2;1:0,0,3;2:0,0,4;3:1,5;4:2;5:3|0|5|3|0->1->3->5"
)

echo "Тестовое окружение:"
echo "  Протокол:  $PROTOCOL"
echo "  Порт:      $PORT"
echo "  Клиент:    $CLIENT"
echo "  Тесты:     ${#TEST_CASES[@]}"
echo ""

PASSED=0
FAILED=0

for test_case in "${TEST_CASES[@]}"; do
    # Разбираем тестовый случай
    IFS='|' read -r test_name vertices edges start end length path <<< "$test_case"

    echo "=============================================="
    echo "Тест: $test_name"
    echo "  Вершин: $vertices"
    echo "  Ребра: $edges"
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

    # Запускаем тест с передачей всех параметров
    if "$EXPECT_SCRIPT" "$CLIENT" "$vertices" "$edges" "$start" "$end" "$length" "$path" "$PROTOCOL" "$PORT"; then
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
echo -e "  Всего тестов: $((PASSED + FAILED))"
echo -e "  ${GREEN}Пройдено:    $PASSED${NC}"
if [ $FAILED -gt 0 ]; then
    echo -e "  ${RED}Провалено:  $FAILED${NC}"
fi

if [ $FAILED -gt 0 ]; then
    exit 1
else
    echo -e "${GREEN}✅ Все тесты пройдены успешно!${NC}"
    exit 0
fi