#include <Python.h>

int main() {
    // Python 인터프리터 초기화
    Py_Initialize();

    // Python 모듈 'openai.py'가 있는 경로를 추가
    PyRun_SimpleString("import sys\n"
                       "sys.path.append('./modules')");

    // Python 스크립트 'openai.py' 불러오기
    PyRun_SimpleString("import openai");

    // 추가적인 Python 코드 실행 (예: openai 모듈의 함수 호출)
    PyRun_SimpleString("openai.some_function()");

    // Python 인터프리터 종료
    Py_Finalize();

    return 0;
}
