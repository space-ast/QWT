#!/usr/bin/env python3
"""
扫描指定目录下的.h头文件，提取带有QWT_EXPORT或QWT3D_EXPORT导出符号的类和结构体
为每个导出的类/结构体生成对应的包含文件
同时处理固定文件和模板类
python make-classinclude.py --scan-dir ../src --output-dir ../classincludes
"""
import os
import re
import argparse
import sys
from pathlib import Path

# 固定文件映射
FIXED_FILES = {
    'QwtMath': 'qwt_math.h',
    'QwtGlobal': 'qwt_global.h'
}

# 需要处理的导出符号列表
EXPORT_SYMBOLS = ['QWT_EXPORT', 'QWT3D_EXPORT']

# 导出符号到文件前缀的映射
EXPORT_PREFIX = {
    'QWT_EXPORT': '',           # QWT_EXPORT -> 不添加前缀
    'QWT3D_EXPORT': 'Qwt3D'     # QWT3D_EXPORT -> 添加Qwt3D前缀
}

def parse_arguments():
    """解析命令行参数"""
    parser = argparse.ArgumentParser(
        description='扫描头文件并生成QWT导出类的包含文件'
    )
    parser.add_argument(
        '--scan-dir', 
        required=True,
        help='要扫描的头文件目录'
    )
    parser.add_argument(
        '--output-dir', 
        required=True,
        help='输出文件目录'
    )
    return parser.parse_args()

def find_header_files(scan_dir):
    """查找目录下所有的.h文件"""
    header_files = []
    scan_path = Path(scan_dir)
    
    if not scan_path.exists():
        print(f"错误: 扫描目录不存在: {scan_dir}")
        return []
    
    for file_path in scan_path.rglob("*.h"):
        header_files.append(file_path)
    
    print(f"找到 {len(header_files)} 个头文件")
    return header_files

def extract_exported_classes(file_path):
    """
    从头文件中提取带有指定导出符号的类和结构体
    返回字典，键为类名，值为导出符号
    只处理EXPORT_SYMBOLS中定义的导出符号
    """
    exported_classes = {}  # 存储类名和对应的导出符号
    
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # 只为指定的导出符号创建正则表达式模式
        for export_symbol in EXPORT_SYMBOLS:
            # 正则表达式匹配带有导出符号的类和结构体
            # 匹配模式: class 导出符号 类名 或 struct 导出符号 结构体名
            # 使用更精确的匹配，确保导出符号紧跟在class/struct后面
            export_patterns = [
                rf'class\s+{export_symbol}\s+(\w+)(?:\s*[:{{<]|\s*$)',  # class EXPORT_SYMBOL ClassName
                rf'struct\s+{export_symbol}\s+(\w+)(?:\s*[:{{<]|\s*$)', # struct EXPORT_SYMBOL StructName
            ]
            
            for pattern in export_patterns:
                matches = re.findall(pattern, content)
                for class_name in matches:
                    # 只添加有效的类名，避免重复
                    if class_name and class_name not in exported_classes:
                        exported_classes[class_name] = export_symbol
                        print(f"    找到导出类: {class_name} 使用 {export_symbol}")
        
        # 可选：处理模板类，但只处理带有导出符号的模板类
        # 模板类的匹配更复杂，需要谨慎处理
        template_patterns = [
            rf'template\s*<[^>]*>\s*class\s+{export_symbol}\s+(\w+)',  # template<...> class EXPORT_SYMBOL ClassName
            rf'template\s*<[^>]*>\s*struct\s+{export_symbol}\s+(\w+)', # template<...> struct EXPORT_SYMBOL StructName
        ]
        
        for export_symbol in EXPORT_SYMBOLS:
            for pattern in template_patterns:
                matches = re.findall(pattern.format(export_symbol=export_symbol), content)
                for class_name in matches:
                    if class_name and class_name not in exported_classes:
                        exported_classes[class_name] = export_symbol
                        print(f"    找到模板导出类: {class_name} 使用 {export_symbol}")
        
    except Exception as e:
        print(f"读取文件 {file_path} 时出错: {e}")
    
    return exported_classes

def generate_output_filename(class_name, export_symbol):
    """
    根据导出符号生成输出文件名
    """
    if export_symbol in EXPORT_PREFIX and EXPORT_PREFIX[export_symbol]:
        # 对于有前缀映射的导出符号，添加前缀
        return f"{EXPORT_PREFIX[export_symbol]}{class_name}"
    else:
        # 对于没有前缀映射的导出符号，直接使用类名
        return class_name

def generate_include_file(output_dir, class_name, export_symbol, header_file_path=None, scan_dir=None, fixed_header=None):
    """
    为每个导出的类生成包含文件
    """
    try:
        # 生成输出文件名
        output_filename = generate_output_filename(class_name, export_symbol)
        output_file = Path(output_dir) / output_filename
        
        if fixed_header:
            # 固定文件，直接使用指定的头文件
            include_content = f'#include "{fixed_header}"\n'
        else:
            # 计算头文件相对于扫描目录的相对路径
            rel_header_path = Path(header_file_path).relative_to(scan_dir)
            include_content = f'#include "{rel_header_path}"\n'
        
        # 写入包含指令
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(include_content)
        
        return True, output_filename
        
    except Exception as e:
        print(f"生成文件 {class_name} 时出错: {e}")
        return False, None

def generate_fixed_files(output_dir, scan_dir):
    """
    生成固定文件
    """
    generated_count = 0
    
    for class_name, header_name in FIXED_FILES.items():
        # 检查头文件是否存在
        header_path = Path(scan_dir) / header_name
        if header_path.exists():
            # 固定文件使用特殊的导出符号标记
            success, output_filename = generate_include_file(
                output_dir, class_name, 'FIXED', fixed_header=header_name
            )
            if success:
                print(f"生成固定文件: {output_filename} -> {header_name}")
                generated_count += 1
        else:
            print(f"警告: 固定文件所需的头文件不存在: {header_name}")
    
    return generated_count

def main():
    args = parse_arguments()
    
    # 检查并创建输出目录
    output_path = Path(args.output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    
    print(f"将处理以下导出符号: {', '.join(EXPORT_SYMBOLS)}")
    
    # 生成固定文件
    print("\n生成固定文件...")
    fixed_count = generate_fixed_files(args.output_dir, args.scan_dir)
    
    # 查找所有头文件
    header_files = find_header_files(args.scan_dir)
    if not header_files:
        print("没有找到头文件，程序退出")
        return
    
    total_classes = fixed_count
    processed_files = 0
    files_with_exports = 0
    
    # 处理每个头文件
    for header_file in header_files:
        print(f"\n处理文件: {header_file}")
        
        # 提取导出的类
        exported_classes = extract_exported_classes(header_file)
        
        if exported_classes:
            files_with_exports += 1
            class_list = list(exported_classes.keys())
            print(f"  找到 {len(exported_classes)} 个导出类: {', '.join(class_list)}")
            
            # 为每个导出的类生成文件
            for class_name, export_symbol in exported_classes.items():
                # 检查是否已经生成了固定文件，避免重复
                if class_name not in FIXED_FILES:
                    success, output_filename = generate_include_file(
                        args.output_dir, class_name, export_symbol, 
                        header_file, args.scan_dir
                    )
                    if success:
                        total_classes += 1
                        print(f"    生成文件: {output_filename} (来自类 {class_name}, 使用 {export_symbol})")
        else:
            print(f"  没有找到带有指定导出符号的类")
        
        processed_files += 1
    
    print(f"\n处理完成!")
    print(f"扫描头文件: {processed_files} 个")
    print(f"包含导出类的头文件: {files_with_exports} 个")
    print(f"生成包含文件: {total_classes} 个")
    print(f"输出目录: {args.output_dir}")

if __name__ == "__main__":
    main()