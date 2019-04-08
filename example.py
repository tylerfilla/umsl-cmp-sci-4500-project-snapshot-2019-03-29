#
# faces example
#

import faces

import cv2
from PIL import Image

counter = 1
rectangles = {}

def on_face_appear(rec: faces.Recognizer, fid: int, bounds: tuple, enc: faces.Encoding):
    """Called when the recognizer notices a new face."""

    global counter
    global rectangles

    rectangles[fid] = bounds

    if fid < 0:
        print(f"I've never seen you before! I'll call you {counter}...")
        rec.cache.rename(fid, counter)
        counter = counter + 1
    elif fid > 0:
        print(f"Hello again {fid}!")


def on_face_disappear(rec: faces.Recognizer, fid: int):
    """Called when the recognizer loses track of a face."""

    global rectangles

    print(f'disappear: {fid}')

    del rectangles[fid]


def on_face_move(rec: faces.Recognizer, fid: int, bounds: tuple):
    """Called when a tracked face moves on camera."""

    global rectangles

    print(f'move: {fid} at {bounds}')
    rectangles[fid] = bounds


def main():
    # Open the camera
    cap = cv2.VideoCapture(0)

    # Demo face 1
    face1 = faces.Encoding()
    face1.vector = [0 for x in range(1, 129)]

    # Set up the face cache
    cache: faces.Cache = faces.caches.BasicCache()

    # Set up the video source
    source: faces.Source = faces.sources.PILSource()

    # Set up the face recognizer
    rec = faces.Recognizer()
    rec.cache = cache
    rec.source = source

    # Register for face event callbacks
    rec.register_face_appear(on_face_appear)
    rec.register_face_disappear(on_face_disappear)
    rec.register_face_move(on_face_move)

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

        # Poll for face event callbacks
        rec.poll()

        # Draw face rectangles
        for id in rectangles:
            # The rectangle for this face
            rect = rectangles[id]

            # Generate a random(-ish) color for this ID
            # This is not cryptographically-secure...
            color = ((id * 2334643 + 1) % 256, (id * 133643 + 1) % 256, (id * 165451 + 1) % 256)

            # Draw the rectangle
            # It may be a few frames old, but that *is* the point of asynchronous face recognition
            cv2.rectangle(frame, (rect[0], rect[1]), (rect[2], rect[3]), color, thickness=3)

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
