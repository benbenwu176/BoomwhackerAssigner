from xlsxwriter import Workbook

def create_player_columns_xlsx(
    output_path: str,
    bg_colors: list[str],
    lines: list[str],
    delimiter: str = "|",
    col_width: int = 22
):
    # Parse: "i|n|c|elem1|elem2|..."
    data = {i: {"notes": "0", "caps": "0", "elements": []} for i in range(len(bg_colors))}
    for raw in lines:
        parts = raw.split(delimiter)
        idx = int(parts[0]); notes = parts[1]; caps = parts[2]
        data[idx] = {"notes": notes, "caps": caps, "elements": parts[3:]}

    max_elements = max(len(data[i]["elements"]) for i in range(len(bg_colors)))

    wb = Workbook(output_path)
    ws = wb.add_worksheet("Players")

    # Column widths
    for col in range(len(bg_colors)):
        ws.set_column(col, col, col_width)

    def title_format(bg_hex):
        return wb.add_format({
            "font_name": "Times New Roman", "font_size": 12, "bold": True,
            "text_wrap": True, "valign": "vcenter", "align": "center",
            "bg_color": bg_hex, "font_color": "#000000",
            "left": 2, "right": 2, "top": 2, "bottom": 2  # thick borders
        })

    def element_format(font_hex, bottom=False):
        base = {
            "font_name": "Times New Roman", "font_size": 10,
            "align": "left", "valign": "vcenter", "font_color": font_hex,
            "left": 2, "right": 2  # column box sides
        }
        if bottom:
            base["bottom"] = 2
        return wb.add_format(base)

    for col_index, bg_hex in enumerate(bg_colors):
        info = data.get(col_index, {"notes": "0", "caps": "0", "elements": []})
        notes, caps, elements = info["notes"], info["caps"], info["elements"]

        # Title (row 0)
        ws.write(0, col_index, f"Player {col_index}\n{notes} notes\n{caps} caps",
                 title_format(bg_hex))
        ws.set_row(0, 54)  # space for 3 lines

        # Elements (rows 1..max_elements)
        for r in range(1, max_elements + 1):
            val = elements[r - 1] if r - 1 < len(elements) else ""
            fmt = element_format(bg_hex, bottom=(r == max_elements))
            ws.write(r, col_index, val, fmt)

    ws.freeze_panes(1, 0)
    wb.close()


colors = ["#FF0000", "#0000FF", "#00FF00"]
lines = [
    "0|3|2|Alice|Bob|Charlie",
    "1|2|1|Xander|Yolanda",
    "2|4|3|Zoe|Yannick|Xavier|William"
]
create_player_columns_xlsx("players.xlsx", colors, lines)
