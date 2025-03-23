#!/bin/zsh
if [[ $# -ne 1 ]]; then
    echo "路径不对"
    echo "./convert path/to/image.jpg"
    exit 1
fi

IMAGE_PATH="$1"
if [[ ! -f "$IMAGE_PATH" ]]; then
    echo "图片\"$IMAGE_PATH\" 不存在"
    exit 1
fi

javac ImageToArray.java
if [[ $? -ne 0 ]]; then
    echo "编译失败"
    exit 1
fi

java ImageToArray "$IMAGE_PATH"
if [[ $? -eq 0 ]]; then
    echo "ok了👌"
else
    echo "转换不成功"
fi
