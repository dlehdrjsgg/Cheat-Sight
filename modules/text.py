import openai
import os
import sys

def load_api_key():
    """Load the OpenAI API key from the config.env file."""
    env_file = "config.env"
    if not os.path.exists(env_file):
        raise FileNotFoundError(f"[ERROR] {env_file} 파일을 찾을 수 없습니다.")
    
    with open(env_file, "r") as file:
        for line in file:
            if line.startswith("OPENAI_API_KEY"):
                return line.strip().split("=")[1]
    
    raise ValueError("[ERROR] config.env 파일에 'OPENAI_API_KEY'가 설정되어 있지 않습니다.")

def analyze_code(file_path):
    """Analyze the code file using OpenAI API."""
    try:
        with open(file_path, "r") as file:
            code_content = file.read()
    except FileNotFoundError:
        raise FileNotFoundError(f"> {file_path} 파일을 찾을 수 없습니다.")
    
    # 간소화된 프롬프트
    prompt = (
        "아래는 C 프로그램 코드입니다. 이 프로그램의 주요 동작 과정을 간결하고 요약하여 "
        "핵심적인 부분만 설명해주세요. 메모리 구조나 프로그램 실행 흐름을 포함하되, "
        "불필요한 세부사항은 생략하고 이해하기 쉽게 작성해주세요:\n\n"
        f"{code_content}"
    )
    
    # Make a request to OpenAI
    response = openai.ChatCompletion.create(
        model="gpt-4",
        messages=[{"role": "system", "content": "당신은 코드를 간결히 설명하는 도우미입니다."},
                  {"role": "user", "content": prompt}],
        temperature=0.5
    )
    
    print(f"[Text-Module] 텍스트 생성 성공.")
    return response['choices'][0]['message']['content']
    #print("[Text-Module] 텍스트 생성 성공. (테스트 로직)")

def save_analysis(output_folder, file_name, analysis):
    """Save the analysis result to a text file."""
    output_file = os.path.join(output_folder, f"{os.path.splitext(file_name)[0]}_analysis.txt")
    with open(output_file, "w") as file:
        file.write(analysis)
    print(f"[Text-Module] 텍스트 저장 완료 : {output_file}")

def main():
    if len(sys.argv) != 3:
        print("[ERROR] 정상적이지 않은 방법으로 모듈이 실행되었습니다. main.c 코드를 통해 실행해주세요.")
        sys.exit(1)
    
    input_file = sys.argv[1]
    output_folder = sys.argv[2]
    
    try:
        api_key = load_api_key()
        openai.api_key = api_key
    except Exception as e:
        print(f"[ERROR] API 키 로드 실패: {e}")
        sys.exit(1)
    
    try:
        print(f"[Text-Module] ChatGPT-4에게 요청 중...")
        analysis = analyze_code(input_file)
        save_analysis(output_folder, os.path.basename(input_file), analysis)
    except Exception as e:
        print(f"[ERROR] 분석 중 오류 발생: {e}")
        sys.exit(1)

if __name__ == "__main__":
    main()
