import cv2
import numpy as np
import paho.mqtt.client as mqtt

# MQTT setup
mqtt_broker = "broker.hivemq.com"  # Public MQTT broker
mqtt_topic = "ball_detection/zone"  # Topic to publish zone information

# Create MQTT client
mqtt_client = mqtt.Client()
mqtt_client.connect(mqtt_broker)
mqtt_client.loop_start()

# ====== SET PHONE CAMERA IP ======
PHONE_IP = "http://100.68.86.118:8080/video"  # Change if IP changes

# Start phone camera stream
cap = cv2.VideoCapture(PHONE_IP)
if not cap.isOpened():
    print("❌ Could not connect to phone camera.")
    exit()

# Target display width (resize for speed)
DISPLAY_WIDTH = 480
disp_height = 600
zone_count = 3  # Number of zones
zone_width = DISPLAY_WIDTH // zone_count  # Width of each zone

while True:
    ret, frame = cap.read()
    if not ret:
        print("❌ Failed to grab frame.")
        break

    # Rotate 90° clockwise
    frame = cv2.rotate(frame, cv2.ROTATE_90_CLOCKWISE)

    # Resize for speed
    frame = cv2.resize(frame, (DISPLAY_WIDTH, disp_height))

    # Convert to HSV
    hsv = cv2.cvtColor(frame, cv2.COLOR_BGR2HSV)

    # Green color range
    lower_green = np.array([40, 70, 70])
    upper_green = np.array([80, 255, 255])
    green_mask = cv2.inRange(hsv, lower_green, upper_green)
    green_mask = cv2.GaussianBlur(green_mask, (7, 7), 2)

    # Find contours
    contours, _ = cv2.findContours(green_mask, cv2.RETR_EXTERNAL, cv2.CHAIN_APPROX_SIMPLE)

    ball_detected = False
    detected_zone = None
    center = None
    radius = 0

    for cnt in contours:
        area = cv2.contourArea(cnt)
        perimeter = cv2.arcLength(cnt, True)
        if area > 300 and perimeter > 0:
            circularity = 4 * np.pi * (area / (perimeter * perimeter))
            if circularity > 0.6:
                (x, y), radius = cv2.minEnclosingCircle(cnt)
                center = (int(x), int(y))
                radius = int(radius)

                # Determine zone
                if x < zone_width:
                    detected_zone = 1
                elif x < 2 * zone_width:
                    detected_zone = 2
                else:
                    detected_zone = 3

                ball_detected = True
                break

    # If ball detected → send message to MQTT
    if ball_detected:
        mqtt_client.publish(mqtt_topic, f"Ball detected in Zone {detected_zone}")

        # Create a blurred version of the frame
        blurred_frame = cv2.GaussianBlur(frame, (15, 15), 0)

        # Create a mask for the ball
        mask = np.zeros_like(frame)
        cv2.circle(mask, center, radius, (0, 255, 0), -1)  # Draw the ball on the mask

        # Combine the blurred background with the mask
        highlighted_frame = np.where(mask == 0, blurred_frame, frame)

        # Draw a circle around the detected ball
        cv2.circle(highlighted_frame, center, radius, (0, 0, 255), 2)  # Red circle

        # Put the zone text on the frame
        cv2.putText(highlighted_frame, f"Zone {detected_zone}", (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 2)
       # print(detected_zone)

        # Show output
        cv2.imshow("Green Ball Zone Detection (Phone Camera)", highlighted_frame)
    else:
        # Show the original frame if no ball is detected
        cv2.imshow("Green Ball Zone Detection (Phone Camera)", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()
mqtt_client.loop_stop()  # Stop the MQTT loop
mqtt_client.disconnect()  # Disconnect from MQTT broker
