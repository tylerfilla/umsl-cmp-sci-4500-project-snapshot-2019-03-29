#
# faces example
#

import faces

import cv2
from PIL import Image


def main():
    # Open the camera
    cap = cv2.VideoCapture(0)

    # Demo face 1
    face1 = faces.Encoding()
    face1.vector = [0 for x in range(1, 129)]

    # Set up the face cache
    cache: faces.Cache = faces.caches.BasicCache()
    cache.insert(1, face1)

    # Set up the video source
    source: faces.Source = faces.sources.PILSource()

    # Set up the face recognizer
    rec = faces.Recognizer()
    rec.cache = cache
    rec.source = source

    # Start recognition
    rec.start()

    while True:
        # Read a video frame
        ret, frame = cap.read()

        # Convert OpenCV frame from BGR to RGB
        cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # Convert OpenCV frame to PIL image
        frame_pil = Image.fromarray(frame)

        # Send PIL image off for processing
        source.update(frame_pil)

        # Show the frame
        cv2.imshow("Output", frame)

        # Stop on Q key
        if cv2.waitKey(1) & 0xFF == ord('q'):
            break

    # Stop recognition
    rec.stop()

    # Close the camera
    cap.release()


if __name__ == '__main__':
    main()
