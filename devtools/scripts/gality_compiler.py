import re
import json
import os
import sys

class GalityCompiler:
    def __init__(self):
        self.nodes = []
        self.start_node = "node_01"

    def compile(self, source_code: str) -> dict:
        self.nodes = []
        lines = source_code.splitlines()

        current_node = None

        def finalize_current_node():
            nonlocal current_node
            if current_node:
                # 若節點含有 choices，標記型別為 choice
                if current_node.get("choices"):
                    current_node["type"] = "choice"
                # 若節點含有 condition，標記型別為 condition
                elif current_node.get("condition"):
                    current_node["type"] = "condition"
                # 若節點含有 mutations 且無 text/choices，標記為 action
                elif current_node.get("mutations") and not current_node.get("text"):
                    current_node["type"] = "action"
                else:
                    current_node["type"] = "dialogue"

                self.nodes.append(current_node)
                current_node = None

        for raw_line in lines:
            line = raw_line.strip()
            # 略過空行與註解
            if not line or line.startswith("//") or line.startswith("#"):
                continue

            # 1. 解析 @start 入口標籤
            if line.startswith("@start"):
                parts = line.split()
                if len(parts) >= 2:
                    self.start_node = parts[1].strip()
                continue

            # 2. 解析節點標籤 [node_id] (排除選項標籤如 ? [選項])
            if line.startswith("[") and line.endswith("]") and not line.startswith("?"):
                finalize_current_node()
                node_id = line[1:-1].strip()
                current_node = {
                    "id": node_id,
                    "type": "dialogue",
                    "speaker": "",
                    "text": "",
                    "bg": "",
                    "bgm": "",
                    "cv": "",
                    "weather": "none",
                    "shake": 0.0,
                    "transition_mask": "",
                    "transition_duration": 1.0,
                    "next": "",
                    "no_skip": False,     # ⚠️ 新增
                    "wait": 0.0           # ⚠️ 新增
                }
                continue

            # 確保有活躍節點可填充屬性
            if not current_node:
                continue

            # 3. 解析指向目標 -> next_node
            if line.startswith("->"):
                target = line[2:].strip()
                current_node["next"] = target
                continue

            # 4. 解析分支選項 ? [選項文字] -> target_node
            if line.startswith("?"):
                choice_match = re.match(r'^\?\s*\[(.*?)\]\s*->\s*([\w\-]+)', line)
                if choice_match:
                    text, target = choice_match.groups()
                    if "choices" not in current_node:
                        current_node["choices"] = []
                    current_node["choices"].append({
                        "text": text.strip(),
                        "target": target.strip()
                    })
                continue

            # 5. 解析變數賦值 $ favorability += 10
            if line.startswith("$"):
                mut_match = re.match(r'^\$\s*(\w+)\s*(\+=|-=|=)\s*(-?\d+)', line)
                if mut_match:
                    var_name, op, val = mut_match.groups()
                    if "mutations" not in current_node:
                        current_node["mutations"] = []
                    current_node["mutations"].append({
                        "var": var_name,
                        "op": op,
                        "val": int(val)
                    })
                continue

            # 6. 解析條件分支 IF favorability >= 80 THEN node_tier_high ELSE node_tier_low
            if line.startswith("IF "):
                cond_match = re.match(r'^IF\s+(\w+)\s*(>=|<=|>|<|==)\s*(-?\d+)\s+THEN\s+(\w+)\s+ELSE\s+(\w+)', line, re.IGNORECASE)
                if cond_match:
                    var_name, op, val, t_next, f_next = cond_match.groups()
                    current_node["condition"] = {
                        "var": var_name,
                        "op": op,
                        "val": int(val),
                        "then": t_next,
                        "else": f_next
                    }
                continue

            # 7. 解析單行屬性 key: value
            # 嚴格匹配白名單屬性，避免把角色名冒號誤判為屬性
            attr_match = re.match(
                r'^(bg|bgm|cv|trans|transition|duration|weather|shake|char|character|char_left|char_center|char_right|active_char|no_skip|wait)\s*[:=]\s*(.*)$',
                line
            )
            if attr_match:
                key, val = attr_match.groups()
                val = val.strip().strip('"')

                if key == "bg":
                    current_node["bg"] = val
                elif key == "bgm":
                    current_node["bgm"] = val
                elif key == "cv":
                    current_node["cv"] = val
                elif key in ("trans", "transition"):
                    current_node["transition_mask"] = val
                elif key == "duration":
                    try:
                        current_node["transition_duration"] = float(val)
                    except ValueError:
                        current_node["transition_duration"] = 1.0
                elif key == "weather":
                    current_node["weather"] = val
                elif key == "shake":
                    try:
                        current_node["shake"] = float(val)
                    except ValueError:
                        current_node["shake"] = 0.0
                elif key in ("char", "character"):
                    current_node["char_center"] = val
                elif key in ("char_left", "char_center", "char_right", "active_char"):
                    current_node[key] = val
                # ⚠️ 新增
                elif key == "no_skip":
                    current_node["no_skip"] = val.lower() in ("true", "1", "yes")
                elif key == "wait":
                    try:
                        current_node["wait"] = float(val)
                    except ValueError:
                        current_node["wait"] = 0.0
                continue

            # 8. 解析對話台詞：說話者: 台詞 (例如 "學姐: 你好！" 或 "旁白: 春天來了。")
            dialogue_match = re.match(r'^([^:：\s]+)\s*[:：]\s*(.*)$', line)
            if dialogue_match:
                speaker, text = dialogue_match.groups()
                current_node["speaker"] = speaker.strip()
                current_node["text"] = text.strip()
                continue

            # 9. 無說話者的純旁白文本
            if current_node["text"]:
                current_node["text"] += "\n" + line
            else:
                current_node["text"] = line

        finalize_current_node()

        return {
            "start": self.start_node,
            "nodes": self.nodes
        }

def main():
    input_path = "assets/scripts/main_story_multi.gality"
    output_path = "assets/scripts/demo_long.json"

    if len(sys.argv) >= 2:
        input_path = sys.argv[1]
    if len(sys.argv) >= 3:
        output_path = sys.argv[2]

    if not os.path.exists(input_path):
        print(f"[Compiler Error] Source file not found: {input_path}")
        return

    with open(input_path, "r", encoding="utf-8") as f:
        source_code = f.read()

    compiler = GalityCompiler()
    result = compiler.compile(source_code)

    os.makedirs(os.path.dirname(output_path), exist_ok=True)
    with open(output_path, "w", encoding="utf-8") as f:
        json.dump(result, f, ensure_ascii=False, indent=2)

    print(f"[GalityCompiler] Successfully compiled '{input_path}' -> '{output_path}'")
    print(f"[GalityCompiler] Generated {len(result['nodes'])} valid AST nodes. Start node: '{result['start']}'")

if __name__ == "__main__":
    main()