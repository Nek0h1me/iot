import cv2
import face_recognition
import numpy as np
import requests
import os

#  Thư mục chứa ảnh gốc
KNOWN_FACES_DIR = "mat_chu_nha"

#  Load tất cả ảnh trong thư mục matchunha
known_encodings = []
known_names = []

for filename in os.listdir(KNOWN_FACES_DIR):
    if filename.endswith(".jpg") or filename.endswith(".png"):
        img_path = os.path.join(KNOWN_FACES_DIR, filename)
        image = face_recognition.load_image_file(img_path)
        encodings = face_recognition.face_encodings(image)

        if len(encodings) > 0:
            known_encodings.append(encodings[0])
            known_names.append(filename)  # Lưu tên file để nhận diện

print(f"🔹 Đã tải {len(known_encodings)} khuôn mặt từ thư mục '{KNOWN_FACES_DIR}'!")

#  Lấy ảnh từ ESP32-CAM
esp_url = "http://172.20.10.2/capture"
response = requests.get(esp_url)

if response.status_code == 200:
    with open("captured.jpg", "wb") as f:
        f.write(response.content)

    # Load ảnh vừa chụp từ ESP32-CAM
    frame = cv2.imread("captured.jpg")
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

    # 🔍 Tìm khuôn mặt trong ảnh mới
    face_locations = face_recognition.face_locations(rgb_frame)
    face_encodings = face_recognition.face_encodings(rgb_frame, face_locations)

    if len(face_encodings) > 0:
        #  So sánh với tất cả khuôn mặt đã lưu
        for i, known_encoding in enumerate(known_encodings):
            match = face_recognition.compare_faces([known_encoding], face_encodings[0])

            if match[0]:
                print(f"✅ Khuôn mặt trùng khớp với {known_names[i]}!")
                break
        else:
            print("❌ Không tìm thấy khuôn mặt trùng khớp!")

    else:
        print("⚠️ Không tìm thấy khuôn mặt trong ảnh!")

else:
    print("🚫 Không thể lấy ảnh từ ESP32-CAM!")
