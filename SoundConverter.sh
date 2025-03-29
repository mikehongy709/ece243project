if [[ $# -lt 1 ]]; then
    echo "路径不对"
    echo "./convert path/to/image.jpg"
    exit 1
fi

for IMAGE_PATH in "$@"; do

    javac SoundToArray.java
    if [[ $? -ne 0 ]]; then
        echo "编译失败"
        exit 1
    fi

    java SoundToArray "$IMAGE_PATH"
    if [[ $? -eq 0 ]]; then
        echo "ok了👌"
    else
        echo "转换不成功"
    fi
done
