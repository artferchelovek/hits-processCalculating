import subprocess
import os
import re
import csv
from statistics import mean

images = [
    {"name": "300x300.png", "pixels": 300*300, "label": "300x300"},
    {"name": "400x400.png", "pixels": 400*400, "label": "400x400"},
    {"name": "500x500.png", "pixels": 500*500, "label": "500x500"},
    {"name": "600x600.png", "pixels": 600*600, "label": "600x600"},
    {"name": "950x950.png", "pixels": 950*950, "label": "950x950"},
    {"name": "2400x2400.png", "pixels": 2400*2400, "label": "2400x2400"}
]
filters = ["1", "2", "3"]
methods = ["1", "2", "3", "4"]

filter_map = {"1": "inversion", "2": "median", "3": "edges"}
method_map = {"1": "Seq", "2": "OMP", "3": "SIMD", "4": "OCL"}
colors = {"Seq": "#FF5733", "OMP": "#33FF57", "SIMD": "#3357FF", "OCL": "#F333FF"}

ITERATIONS = 3
all_results = []

def generate_svg(filter_name, results):
    width = 900
    height = 500
    padding_left = 80
    padding_right = 140
    padding_top = 50
    padding_bottom = 80
    
    f_results = [r for r in results if r["filter"] == filter_name]
    if not f_results: return ""
    
    max_time = max(r["avg_time"] for r in f_results)
    if max_time == 0: max_time = 1
    
    svg = f'<svg width="{width}" height="{height}" xmlns="http://www.w3.org/2000/svg">\n'
    svg += f'<rect width="100%" height="100%" fill="#ffffff"/>\n'
    svg += f'<text x="{width/2}" y="30" font-family="Arial" font-size="20" font-weight="bold" text-anchor="middle">{filter_name.upper()} FILTER PERFORMANCE</text>\n'
    
    # Draw grid and axes
    svg += f'<line x1="{padding_left}" y1="{height-padding_bottom}" x2="{width-padding_right}" y2="{height-padding_bottom}" stroke="black" stroke-width="2"/>\n'
    svg += f'<line x1="{padding_left}" y1="{padding_top}" x2="{padding_left}" y2="{height-padding_bottom}" stroke="black" stroke-width="2"/>\n'
    
    # Axis labels
    svg += f'<text x="{width-padding_right + 10}" y="{height-padding_bottom + 5}" font-family="Arial" font-size="12">Pixels</text>\n'
    svg += f'<text x="{padding_left - 10}" y="{padding_top - 15}" font-family="Arial" font-size="12" text-anchor="end">Time (ms)</text>\n'

    # Y-axis ticks and labels
    num_ticks_y = 10
    for i in range(num_ticks_y + 1):
        y_val = i * max_time / num_ticks_y
        y_pos = (height - padding_bottom) - (i * (height - padding_top - padding_bottom) / num_ticks_y)
        svg += f'<line x1="{padding_left-5}" y1="{y_pos}" x2="{padding_left}" y2="{y_pos}" stroke="black" stroke-width="1"/>\n'
        svg += f'<line x1="{padding_left}" y1="{y_pos}" x2="{width-padding_right}" y2="{y_pos}" stroke="#eee" stroke-width="1"/>\n'
        svg += f'<text x="{padding_left-10}" y="{y_pos+5}" font-family="Arial" font-size="10" text-anchor="end">{y_val:.1f}</text>\n'

    # X-axis ticks and labels
    for i, img in enumerate(images):
        x_pos = padding_left + (i * (width - padding_left - padding_right) / (len(images)-1))
        svg += f'<line x1="{x_pos}" y1="{height-padding_bottom}" x2="{x_pos}" y2="{height-padding_bottom+5}" stroke="black" stroke-width="1"/>\n'
        svg += f'<text x="{x_pos}" y="{height-padding_bottom+20}" font-family="Arial" font-size="10" text-anchor="middle" transform="rotate(30, {x_pos}, {height-padding_bottom+20})">{img["label"]}</text>\n'

    # Legend
    for i, (m, color) in enumerate(colors.items()):
        svg += f'<rect x="{width-padding_right+20}" y="{padding_top + i*25}" width="20" height="15" fill="{color}"/>\n'
        svg += f'<text x="{width-padding_right+45}" y="{padding_top + 12 + i*25}" font-family="Arial" font-size="12">{m}</text>\n'

    # Plot lines
    for method in method_map.values():
        m_results = [r for r in f_results if r["method"] == method]
        m_results.sort(key=lambda x: x["pixels"])
        
        points = []
        for i, r in enumerate(m_results):
            x = padding_left + (i * (width - padding_left - padding_right) / (len(images)-1))
            y = (height - padding_bottom) - (r["avg_time"] * (height - padding_top - padding_bottom) / max_time)
            points.append(f"{x},{y}")
            svg += f'<circle cx="{x}" cy="{y}" r="4" fill="{colors[method]}"/>\n'
        
        svg += f'<polyline points="{" ".join(points)}" fill="none" stroke="{colors[method]}" stroke-width="3"/>\n'

    svg += '</svg>'
    return svg

print(f"Running benchmarks...")
for img in images:
    img_name = img["name"]
    img_idx = images.index(img) + 1
    for f_idx in filters:
        for m_idx in methods:
            times = []
            for _ in range(ITERATIONS):
                input_str = f"{img_idx}\n{f_idx}\n{m_idx}\n"
                try:
                    process = subprocess.Popen(['./cmake-build-debug/parallel_calculate'], 
                                             stdin=subprocess.PIPE, 
                                             stdout=subprocess.PIPE, 
                                             stderr=subprocess.PIPE,
                                             text=True)
                    stdout, _ = process.communicate(input=input_str, timeout=60)
                    time_match = re.search(r"Время обработки: ([\d.]+) ms", stdout)
                    if time_match:
                        times.append(float(time_match.group(1)))
                except Exception as e:
                    print(f"Error: {e}")
            
            if times:
                avg_time = mean(times)
                all_results.append({
                    "pixels": img["pixels"],
                    "filter": filter_map[f_idx],
                    "method": method_map[m_idx],
                    "avg_time": avg_time
                })

# Generate SVGs and README
for f_name in filter_map.values():
    with open(f"chart_{f_name}.svg", "w") as f:
        f.write(generate_svg(f_name, all_results))

with open("README.md", "w") as f:
    f.write("# Лабораторная работа: Параллельные вычисления (M4 Pro)\n\n")
    f.write("## 1. Сводная таблица производительности\n")
    f.write("| Изображение | Фильтр | Метод | Среднее время (ms) |\n")
    f.write("| :--- | :--- | :--- | :--- |\n")
    for r in all_results:
        f.write(f"| {r['pixels']} px | {r['filter']} | {r['method']} | {r['avg_time']:.4f} |\n")
    
    f.write("\n## 2. Графики производительности\n")
    f.write("### Инверсия\n![Inversion Chart](chart_inversion.svg)\n")
    f.write("### Медианный фильтр\n![Median Chart](chart_median.svg)\n")
    f.write("### Обнаружение границ\n![Edges Chart](chart_edges.svg)\n")
    
    f.write("\n## 3. Выводы\n")
    f.write("- **OpenCL** показывает лучшие результаты на больших данных благодаря разовой инициализации.\n")
    f.write("- **SIMD** эффективен для простых потоковых операций.\n")
    f.write("- **OpenMP** обеспечивает стабильное масштабирование на CPU.\n")
