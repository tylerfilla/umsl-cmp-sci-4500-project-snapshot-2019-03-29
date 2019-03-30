#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

import asyncio
import facelib
import functools

import PIL
import cozmo


class TaskRun:
    """
    A task to run the Cozmo program.
    """

    def __init__(self, num_cozmos):
        self.num_cozmos = num_cozmos

    def run(self):
        # Get an event loop
        loop = asyncio.get_event_loop()

        # Coroutines we must wait on as part of the task
        coroutines = []

        # Connect the Cozmos asynchronously
        # For each Cozmo, we spin up a coroutine and add it to the list above
        for i in range(self.num_cozmos):
            conn = cozmo.connect_on_loop(loop)
            coroutines.append(asyncio.ensure_future(self._client_program(conn), loop=loop))

        # Wait on all the established coroutines
        loop.run_until_complete(asyncio.gather(*coroutines))

    @staticmethod
    async def _client_program(conn: cozmo.conn.CozmoConnection) -> None:
        """
        The client program.

        :param conn: The SDK connection
        """

        # Wait for the robot to be ready, and then get it
        robot: cozmo.robot.Robot = await conn.wait_for_robot()

        # Enable color imaging on the robot
        robot.camera.color_image_enabled = True
        robot.camera.image_stream_enabled = True

        # Listen for incoming camera frames
        robot.add_event_handler(cozmo.camera.EvtNewRawCameraImage,
                                functools.partial(TaskRun._robot_on_new_raw_camera_image, robot))

        # Make a face recognizer for this robot (see facelib.cpp for impl)
        # noinspection PyUnresolvedReferences
        face_recognizer = facelib.FaceRecognizer()
        face_recognizer.on_face_show(functools.partial(TaskRun._robot_on_face_show, robot))
        face_recognizer.on_face_hide(functools.partial(TaskRun._robot_on_face_hide, robot))
        face_recognizer.on_face_move(functools.partial(TaskRun._robot_on_face_move, robot))
        robot.our_stowaway_face_recognizer = face_recognizer

        # Main robot loop
        while True:
            # Poll the face recognizer
            face_recognizer.poll()

            # Sleep for a bit
            await asyncio.sleep(0.1)

    @staticmethod
    def _robot_on_new_raw_camera_image(robot: cozmo.robot.Robot, evt: cozmo.camera.EvtNewRawCameraImage, **kw):
        # Get the new frame
        frame: PIL.Image.Image = evt.image

        # Get frame bounding box
        # This is a tuple like follows: (left, top, right, bottom)
        # We'll use it to compute the frame's width and height
        frame_bbox = frame.getbbox()

        # Useful frame info
        # This includes its raw image content and its dimensions
        frame_bytes = frame.tobytes()
        frame_width = frame_bbox[2] - frame_bbox[0]
        frame_height = frame_bbox[3] - frame_bbox[1]

        # Dump the frame to the face recognizer
        # This will return pretty much immediately, as the facelib C++ code is multithreaded
        # This asynchronous behavior allows the Cozmo SDK event loop to keep chugging away without stopping for faces
        # noinspection PyUnresolvedReferences
        robot.our_stowaway_face_recognizer.submit_frame(frame_bytes, frame_width, frame_height)

    @staticmethod
    def _robot_on_face_show(robot: cozmo.robot.Robot, fid: int, x: int, y: int, width: int, height: int) -> None:
        print(f'face {fid} show: {x} {y} {width} {height}')

    @staticmethod
    def _robot_on_face_hide(robot: cozmo.robot.Robot, fid: int, x: int, y: int, width: int, height: int) -> None:
        print(f'face {fid} hide: {x} {y} {width} {height}')

    @staticmethod
    def _robot_on_face_move(robot: cozmo.robot.Robot, fid: int, x: int, y: int, width: int, height: int) -> None:
        print(f'face {fid} move: {x} {y} {width} {height}')


# Make Cozmo stay on the charger when we connect
cozmo.robot.Robot.drive_off_charger_on_connect = False
