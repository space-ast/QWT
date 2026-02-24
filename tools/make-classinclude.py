#!/usr/bin/env python3
"""
扫描指定目录下的.h 头文件，提取带有 QWT_EXPORT 或 QWT3D_EXPORT 导出符号的类和结构体
为每个导出的类/结构体生成对应的包含文件
同时处理固定文件和模板类

默认行为：
- output_path: 脚本所在目录下的 classincludes 文件夹
- scan_dir: 脚本所在目录的上级目录的 src 文件夹
- 扫描两个子目录：src/plot 和 src/plot3d

python make-classinclude.py --scan-dir ../src --output-dir ../classincludes
"""
import os
import re
import argparse
import sys
from pathlib import Path

# 固定文件映射 - 指定在哪个子目录中查找
FIXED_FILES = {
    'QwtMath': {'header': '../plot/qwt_math.h', 'subdir': 'plot'},
    'QwtGlobal': {'header': '../plot/qwt_global.h', 'subdir': 'plot'}
}

# 需要处理的导出符号列表
EXPORT_SYMBOLS = ['QWT_EXPORT', 'QWT3D_EXPORT']

# 导出符号到文件前缀的映射
EXPORT_PREFIX = {
    'QWT_EXPORT': '',           # QWT_EXPORT -> 不添加前缀
    'QWT3D_EXPORT': 'Qwt3D'     # QWT3D_EXPORT -> 添加 Qwt3D 前缀
}

# 扫描目录配置
SCAN_DIRS_CONFIG = {
    'plot': {
        'path': 'plot',
        'include_prefix': '../plot/',
        'export_symbols': ['QWT_EXPORT'],
        'class_prefix': 'qwt_'
    },
    'plot3d': {
        'path': 'plot3d',
        'include_prefix': '../plot3d/',
        'export_symbols': ['QWT3D_EXPORT'],
        'class_prefix': 'qwt3d_'
    }
}


def parse_arguments():
    """解析命令行参数"""
    # 获取脚本所在目录
    script_dir = Path(__file__).parent.resolve()
    
    # 默认参数
    default_scan_dir = script_dir.parent / 'src'
    default_output_dir = script_dir.parent / 'classincludes'
    
    parser = argparse.ArgumentParser(
        description='扫描头文件并生成 QWT 导出类的包含文件'
    )
    parser.add_argument(
        '--scan-dir',
        default=str(default_scan_dir),
        help=f'要扫描的头文件目录 (默认：{default_scan_dir})'
    )
    parser.add_argument(
        '--output-dir',
        default=str(default_output_dir),
        help=f'输出文件目录 (默认：{default_output_dir})'
    )
    return parser.parse_args()


def find_header_files(scan_dir):
    """查找目录下所有的.h 文件"""
    header_files = []
    scan_path = Path(scan_dir)
    if not scan_path.exists():
        print(f"错误：扫描目录不存在：{scan_dir}")
        return []
    for file_path in scan_path.rglob("*.h"):
        header_files.append(file_path)
    print(f"找到 {len(header_files)} 个头文件")
    return header_files


def extract_exported_classes(file_path, export_symbols):
    """
    从头文件中提取带有指定导出符号的类和结构体
    返回字典，键为类名，值为导出符号
    只处理指定的导出符号
    """
    exported_classes = {}  # 存储类名和对应的导出符号
    try:
        with open(file_path, 'r', encoding='utf-8', errors='ignore') as f:
            content = f.read()
        
        # 只为指定的导出符号创建正则表达式模式
        for export_symbol in export_symbols:
            # 正则表达式匹配带有导出符号的类和结构体
            # 匹配模式：class 导出符号 类名 或 struct 导出符号 结构体名
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
                        print(f"    找到导出类：{class_name} 使用 {export_symbol}")
            
            # 处理模板类
            template_patterns = [
                rf'template\s*<[^>]*>\s*class\s+{export_symbol}\s+(\w+)',  # template<...> class EXPORT_SYMBOL ClassName
                rf'template\s*<[^>]*>\s*struct\s+{export_symbol}\s+(\w+)', # template<...> struct EXPORT_SYMBOL StructName
            ]
            for pattern in template_patterns:
                matches = re.findall(pattern, content)
                for class_name in matches:
                    if class_name and class_name not in exported_classes:
                        exported_classes[class_name] = export_symbol
                        print(f"    找到模板导出类：{class_name} 使用 {export_symbol}")
    except Exception as e:
        print(f"读取文件 {file_path} 时出错：{e}")
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


def generate_include_file(output_dir, class_name, export_symbol, header_file_path=None, 
                          scan_dir=None, fixed_header=None, include_prefix=None):
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
            # 使用指定的包含前缀路径
            if include_prefix:
                # 获取头文件名
                header_name = Path(header_file_path).name
                include_content = f'#include "{include_prefix}{header_name}"\n'
            else:
                # 计算头文件相对于扫描目录的相对路径
                rel_header_path = Path(header_file_path).relative_to(scan_dir)
                include_content = f'#include "{rel_header_path}"\n'
        
        # 写入包含指令
        with open(output_file, 'w', encoding='utf-8') as f:
            f.write(include_content)
        return True, output_filename
    except Exception as e:
        print(f"生成文件 {class_name} 时出错：{e}")
        return False, None


def generate_fixed_files(output_dir, base_scan_dir):
    """
    生成固定文件
    修改：支持指定子目录查找固定文件
    """
    generated_count = 0
    for class_name, file_info in FIXED_FILES.items():
        header_name = file_info['header']
        subdir = file_info.get('subdir', '')
        
        # 检查头文件是否存在 - 支持子目录
        if subdir:
            header_path = Path(base_scan_dir) / subdir / header_name
        else:
            header_path = Path(base_scan_dir) / header_name
            
        if header_path.exists():
            # 固定文件使用特殊的导出符号标记
            success, output_filename = generate_include_file(
                output_dir, class_name, 'FIXED', fixed_header=header_name
            )
            if success:
                print(f"生成固定文件：{output_filename} -> {header_path}")
                generated_count += 1
        else:
            print(f"警告：固定文件所需的头文件不存在：{header_path}")
    return generated_count


def scan_directory(scan_dir, output_dir, dir_config):
    """
    扫描指定目录并生成包含文件
    """
    print(f"\n{'='*60}")
    print(f"扫描目录：{scan_dir}")
    print(f"导出符号：{', '.join(dir_config['export_symbols'])}")
    print(f"包含前缀：{dir_config['include_prefix']}")
    print(f"{'='*60}")
    
    header_files = find_header_files(scan_dir)
    if not header_files:
        print("没有找到头文件")
        return 0, 0, 0
    
    total_classes = 0
    processed_files = 0
    files_with_exports = 0
    
    # 处理每个头文件
    for header_file in header_files:
        print(f"\n处理文件：{header_file}")
        
        # 提取导出的类
        exported_classes = extract_exported_classes(header_file, dir_config['export_symbols'])
        
        if exported_classes:
            files_with_exports += 1
            class_list = list(exported_classes.keys())
            print(f"  找到 {len(exported_classes)} 个导出类：{', '.join(class_list)}")
            
            # 为每个导出的类生成文件
            for class_name, export_symbol in exported_classes.items():
                # 检查是否已经生成了固定文件，避免重复
                if class_name not in FIXED_FILES:
                    success, output_filename = generate_include_file(
                        output_dir, class_name, export_symbol,
                        header_file, scan_dir,
                        include_prefix=dir_config['include_prefix']
                    )
                    if success:
                        total_classes += 1
                        print(f"    生成文件：{output_filename} (来自类 {class_name}, 使用 {export_symbol})")
        else:
            print(f"  没有找到带有指定导出符号的类")
        
        processed_files += 1
    
    return processed_files, files_with_exports, total_classes


def main():
    args = parse_arguments()
    
    # 检查并创建输出目录
    output_path = Path(args.output_dir)
    output_path.mkdir(parents=True, exist_ok=True)
    print(f"输出目录：{output_path}")
    
    # 基础扫描目录
    base_scan_dir = Path(args.scan_dir)
    if not base_scan_dir.exists():
        print(f"错误：基础扫描目录不存在：{base_scan_dir}")
        return
    
    total_processed_files = 0
    total_files_with_exports = 0
    total_classes = 0
    
    # 生成固定文件（只生成一次）
    print("\n生成固定文件...")
    fixed_count = generate_fixed_files(output_path, base_scan_dir)
    total_classes += fixed_count
    
    # 扫描配置的各个子目录
    for dir_name, dir_config in SCAN_DIRS_CONFIG.items():
        scan_dir = base_scan_dir / dir_config['path']
        if scan_dir.exists():
            processed, with_exports, classes = scan_directory(
                scan_dir, output_path, dir_config
            )
            total_processed_files += processed
            total_files_with_exports += with_exports
            total_classes += classes
        else:
            print(f"\n警告：目录不存在，跳过：{scan_dir}")
    
    # 打印统计信息
    print(f"\n{'='*60}")
    print("处理完成!")
    print(f"{'='*60}")
    print(f"扫描头文件：{total_processed_files} 个")
    print(f"包含导出类的头文件：{total_files_with_exports} 个")
    print(f"生成包含文件：{total_classes} 个")
    print(f"输出目录：{output_path}")


if __name__ == "__main__":
    main()