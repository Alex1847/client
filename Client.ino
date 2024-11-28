#include <WiFi.h>
#include <esp_now.h>
#include <Update.h>
String currentVersion = "0.9.0";  // 현재 클라이언트 버전
const size_t packetSize = 240;
// 플래시 메모리에 데이터 쓰기 시 오류를 기록하는 함수
void logError(const char* message) {
    Serial.println(message);
}
// ESP-NOW 데이터 수신 콜백
void onReceive(const uint8_t *mac_addr, const uint8_t *data, int len) {
    static bool updateInProgress = false;
    if (!updateInProgress) {
        // 업데이트 시작
        if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
            logError("Failed to start update");
            return;
        }
        updateInProgress = true;
    }
    // 플래시에 데이터 쓰기
    if (Update.write(data, len) != len) {
        logError("Error writing update data");
        return;
    }
    // 업데이트 완료 확인
    if (Update.isFinished()) {
        if (Update.end(true)) {
            Serial.println("Update complete!");
            ESP.restart();  // 펌웨어 업데이트 후 재부팅
        } else {
            logError("Update failed!");
            updateInProgress = false;  // 실패 시 업데이트 초기화
        }
    }
}
void setup() {
    Serial.begin(115200);
    // Wi-Fi 모드 설정 (필요하지 않지만 ESP-NOW 초기화에 필요)
    WiFi.mode(WIFI_STA);
    // ESP-NOW 초기화
    if (esp_now_init() != ESP_OK) {
        logError("Error initializing ESP-NOW");
        return;
    }
    esp_now_register_recv_cb(onReceive);  // 데이터 수신 콜백 등록
    // 마스터에게 버전 정보를 전송
    uint8_t masterMac[] = {0x24, 0x0A, 0xC4, 0xXX, 0xXX, 0xXX};  // 마스터 MAC 주소
    esp_now_peer_info_t peerInfo;
    memcpy(peerInfo.peer_addr, masterMac, 6);
    peerInfo.channel = 0;
    peerInfo.encrypt = false;
    if (esp_now_add_peer(&peerInfo) != ESP_OK) {
        logError("Failed to add peer");
        return;
    }
    // 버전 정보 전송
    if (esp_now_send(masterMac, (uint8_t*)currentVersion.c_str(), currentVersion.length()) != ESP_OK) {
        logError("Error sending version to master");
    } else {
        Serial.println("Version sent to master.");
    }
}
void loop() {
    // 클라이언트의 주 작업 (추가 기능이 필요하면 여기에 구현)
    Serial.println("Waiting for firmware update...");
    delay(5000);
}