#
# InsertProjectName
# Copyright (c) 2019 The InsertProjectName Contributors
# InsertLicenseText
#

import asyncio
import facelib
import functools
from datetime import datetime

import PIL
import cozmo

from cozmonaut.friend import Friend, FriendDB


class TaskRun:
    """
    A task to run the Cozmo program.
    """

    def __init__(self, num_cozmos):
        self.num_cozmos = num_cozmos

        # Create friend database
        self.friend_db = FriendDB()

        # Add some friends to test with
        # TODO: Load friends from an actual data store (FS or DB)
        self.friend_db.add(
            Friend(
                fid=36,
                name="Tyler",
                photo=PIL.Image.open('me.png'),
                when_first_seen=datetime.now(),
                when_last_seen=datetime.now(),
            )
        )
        self.friend_db.add(
            Friend(
                fid=123,
                name="Rando",
                photo=PIL.Image.open('notme.jpg'),
                when_first_seen=datetime.now(),
                when_last_seen=datetime.now(),
            )
        )

        # Create facelib face registry
        # noinspection PyUnresolvedReferences
        self.face_registry = facelib.Registry()

        # Go over all friends we know
        # These friends are shared for all the Cozmos
        for fid in self.friend_db.list():
            # Get this friend
            friend = self.friend_db.get(fid)

            # Add face to face registry
            # We use the friend ID as the face ID (since a human has only one face)
            # noinspection PyUnresolvedReferences
            image = facelib.Image()
            image.width = friend.photo.getbbox()[2] - friend.photo.getbbox()[0]
            image.height = friend.photo.getbbox()[3] - friend.photo.getbbox()[1]
            image.bytes = friend.photo.tobytes()
            self.face_registry.add_face(fid, image)

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

    async def _client_program(self, conn: cozmo.conn.CozmoConnection) -> None:
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
        face_recognizer = facelib.Recognizer()
        face_recognizer.registry = self.face_registry
        face_recognizer.on_face_show(functools.partial(TaskRun._robot_on_face_show, robot))
        face_recognizer.on_face_hide(functools.partial(TaskRun._robot_on_face_hide, robot))
        face_recognizer.on_face_move(functools.partial(TaskRun._robot_on_face_move, robot))
        face_recognizer.start_processor()

        # Store the face recognizer in the robot
        # This is where dynamic languages come in handy
        robot.our_stowaway_face_recognizer = face_recognizer

        # Main robot loop
        while True:
            # Poll the face recognizer
            # This calls our callbacks for any enqueued face-related events
            face_recognizer.poll()

            # Sleep for a bit
            # This lets the other coroutines have a go
            await asyncio.sleep(0.01)

    @staticmethod
    def _robot_on_new_raw_camera_image(robot: cozmo.robot.Robot, evt: cozmo.camera.EvtNewRawCameraImage, **kw):
        # Get the new frame in PIL format
        pil_frame: PIL.Image.Image = evt.image

        # Get frame bounding box
        # This is a tuple like follows: (left, top, right, bottom)
        # We'll use it to compute the frame's width and height
        pil_frame_bbox = pil_frame.getbbox()

        # Convert frame to facelib format
        # noinspection PyUnresolvedReferences
        frame = facelib.Image()
        frame.width = pil_frame_bbox[2] - pil_frame_bbox[0]
        frame.height = pil_frame_bbox[3] - pil_frame_bbox[1]
        frame.bytes = pil_frame.tobytes()

        # Dump the frame to the face recognizer
        # This will return pretty much immediately, as the facelib C++ code is multithreaded
        # This asynchronous behavior allows the Cozmo SDK event loop to keep chugging away without stopping for faces
        # noinspection PyUnresolvedReferences
        robot.our_stowaway_face_recognizer.submit_frame(frame)

    # noinspection PyUnresolvedReferences
    @staticmethod
    def _robot_on_face_show(robot: cozmo.robot.Robot, evt: facelib.Event) -> None:
        print(f'face {evt.fid} show: {evt.x} {evt.y} {evt.width} {evt.height}')

    # noinspection PyUnresolvedReferences
    @staticmethod
    def _robot_on_face_hide(robot: cozmo.robot.Robot, evt: facelib.Event) -> None:
        print(f'face {evt.fid} hide: {evt.x} {evt.y} {evt.width} {evt.height}')

    # noinspection PyUnresolvedReferences
    @staticmethod
    def _robot_on_face_move(robot: cozmo.robot.Robot, evt: facelib.Event) -> None:
        print(f'face {evt.fid} move: {evt.x} {evt.y} {evt.width} {evt.height}')


# Make Cozmo stay on the charger when we connect
cozmo.robot.Robot.drive_off_charger_on_connect = False
