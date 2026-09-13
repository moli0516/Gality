import sys
import json
import re

def parse_gality(file_path):
    with open(file_path, 'r', encoding='utf-8') as f:
        lines = f.readlines()

    start_node = "start"
    nodes = []
    current_node = None

    for raw_line in lines:
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue

        # 解析 @start 指令
        if line.startswith("@start"):
            start_node = line.split()[1]
            continue

        # 解析 [node_id] 標籤
        if line.startswith("[") and line.endswith("]"):
            if current_node:
                nodes.append(current_node)
            node_id = line[1:-1]
            current_node = {
                "id": node_id,
                "type": "dialogue",
                "speaker": "",
                "text": "",
                "bg": "",
                "character": "",
                "bgm": "",
                "cv": "",
                "next": ""
            }
            continue

        if not current_node:
            continue

        # 解析資源屬性
        if line.startswith("bg:"):
            current_node["bg"] = line.split(":", 1)[1].strip()
        elif line.startswith("char:"):
            current_node["character"] = line.split(":", 1)[1].strip()
        elif line.startswith("bgm:"):
            current_node["bgm"] = line.split(":", 1)[1].strip()
        elif line.startswith("cv:"):
            current_node["cv"] = line.split(":", 1)[1].strip()
        
        # 解析單向鏈結 -> node_id
        elif line.startswith("->"):
            current_node["next"] = line.split("->")[1].strip()

        # 解析 Action 指令: $ flag += val
        elif line.startswith("$"):
            match = re.match(r"\$\s*(\w+)\s*\+=\s*(-?\d+)", line)
            if match:
                current_node["type"] = "action"
                current_node["flag"] = match.group(1)
                current_node["value"] = int(match.group(2))

        # 解析 Condition 指令: IF flag >= val THEN true_node ELSE false_node
        elif line.startswith("IF"):
            match = re.match(r"IF\s+(\w+)\s*>=\s*(\d+)\s+THEN\s+(\w+)\s+ELSE\s+(\w+)", line)
            if match:
                current_node["type"] = "condition"
                current_node["condition"] = {
                    "flag": match.group(1),
                    "value": int(match.group(2)),
                    "trueNext": match.group(3),
                    "falseNext": match.group(4)
                }

        # 解析 Choice 指令: ? [選項] -> node_id
        elif line.startswith("?"):
            current_node["type"] = "choice"
            if "choices" not in current_node:
                current_node["choices"] = []
            
            match = re.match(r"\?\s*\[(.*?)\]\s*->\s*(\w+)", line)
            if match:
                current_node["choices"].append({
                    "text": match.group(1),
                    "next": match.group(2)
                })

        # 解析對話內容: Speaker: Text
        elif ":" in line or "：" in line:
            sep = ":" if ":" in line else "："
            parts = line.split(sep, 1)
            current_node["speaker"] = parts[0].strip()
            current_node["text"] = parts[1].strip()

    if current_node:
        nodes.append(current_node)

    # 封裝成標準 JSON 結構
    return {
        "startNode": start_node,
        "nodes": nodes
    }

def main():
    if len(sys.argv) < 3:
        print("Usage: python gality_compiler.py <input.gality> <output.json>")
        return

    input_file = sys.argv[1]
    output_file = sys.argv[2]

    result = parse_gality(input_file)
    with open(output_file, 'w', encoding='utf-8') as f:
        json.dump(result, f, ensure_ascii=False, indent=2)

    print(f"[Gality DSL Compiler] Successfully compiled {input_file} -> {output_file}")

if __name__ == "__main__":
    main()