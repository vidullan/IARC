import numpy as np
import cv2
import imutils
import apriltag
import math

def magnitude(v1, v2):
    return math.sqrt((v2[1]-v1[1])**2 + (v2[0]-v1[0])**2)

class MyVideoCapture:

    def __init__(self, video_source=0):
        self.vid = cv2.VideoCapture(video_source)
        self.video_source = video_source
        if not self.vid.isOpened():
            raise ValueError("Unable to open video source", video_source)

        # Get video source width and height
        self.width = self.vid.get(cv2.CAP_PROP_FRAME_WIDTH)
        self.height = self.vid.get(cv2.CAP_PROP_FRAME_HEIGHT)

    # Release the video source when the object is destroyed
    def __del__(self):
        if self.vid.isOpened():
            self.vid.release()
            #self.window.mainloop()

    def get_frame(self):
        if self.vid.isOpened():
            ret, frame = self.vid.read()
            if ret:
                # Return a boolean success flag and the current frame converted to BGR
                return (ret, frame) #cv2.cvtColor(frame, cv2.COLOR_BGR2RGB))
            else:
                return (ret, None)
        else:
            return (ret, None)

class ShapeDetector:
	def __init__(self):
		pass

	def detect(self, c):
		# initialize the shape name and approximate the contour
		shape = "unidentified"
		peri = cv2.arcLength(c, True)
		approx = cv2.approxPolyDP(c, 0.04 * peri, True)

		if len(approx) == 4:
			# compute the bounding box of the contour and use the
			# bounding box to compute the aspect ratio
			(x, y, w, h) = cv2.boundingRect(approx)
			ar = w / float(h)

			# a square will have an aspect ratio that is approximately
			# equal to one, otherwise, the shape is a rectangle
			shape = "square" if ar >= 0.9 and ar <= 1.1 else "rectangle"

		# return the name of the shape
		return shape

class ImageProcessing:

    def __init__(self, FLAG_ARRAY=[False, False, False, False, False]):

        self.HOME_BASE_FLAG = FLAG_ARRAY[0]
        self.FERRY_FLAG = FLAG_ARRAY[1]
        self.DROP_OFF_FLAG = FLAG_ARRAY[2]
        self.PICK_UP_FLAG = FLAG_ARRAY[3]
        self.BORDER_FLAG = FLAG_ARRAY[4]

        self.LOCATE_TAG_FLAG = False
        self.OVER_TAG_FLAG = False

        self.video = MyVideoCapture(0)
        self.width = int(self.video.width)
        self.height = int(self.video.height)
        self.quad_x = int(self.height/2)
        self.quad_y = int(self.width/2)

        self.theta = math.radians(70) # width angle
        self.psi = math.radians(30) # height angle
        self.pix_to_meter = 0

        self.altitude = 1
        self.x = -1
        self.y = -1
        self.tolerance = 100

        options = apriltag.DetectorOptions(families='tag36h11',
                                           border=1,
                                           nthreads=1,
                                           quad_decimate=1.0,
                                           quad_blur=0.0,
                                           refine_edges=True,
                                           refine_decode=True,
                                           refine_pose=True,
                                           debug=False,
                                           quad_contours=True)

        self.detector = apriltag.Detector(options)

    def print_flags(self):

        print(f"HOME_BASE_FLAG = {self.HOME_BASE_FLAG}\nFERRY_FLAG = {self.FERRY_FLAG}\nDROP_OFF_FLAG = {self.DROP_OFF_FLAG}\nPICK_UP_FLAG {self.PICK_UP_FLAG}\nBORDER_FLAG {self.BORDER_FLAG}\n")

    def update_setpoint(self, x, y):

        if x == -1 or y == -1:
            return

        self.x = x
        self.y = y

    def compute_pixel_dist(self):
        width = 2*self.altitude*math.tan(theta/2)
        self.pix_to_meter = width/self.width

    def process_images(self):

        while True:
            ret, frame = self.video.get_frame()
            output = frame.copy()

            self.compute_pixel_dist()

            if not self.LOCATE_TAG_FLAG:
                output, (x,y) = self.detect_apriltag(frame)
                if x != -1 and y != -1:
                    self.LOCATE_TAG_FLAG = True
            """
            pix_dist = magnitude((self.quad_x, self.quad_y), (x,y))
            if pix_dist < 500:
                self.LOCATE_TAG_FLAG = False
                self.OVER_TAG_FLAG = True
            """
            if self.DROP_OFF_FLAG and self.OVER_TAG_FLAG:
                output, (x,y) = self.detect_drop_off(frame)

            if self.BORDER_FLAG and self.OVER_TAG_FLAG:
                output, (x,y) = self.detect_border(frame)

            if self.PICK_UP_FLAG and self.OVER_TAG_FLAG:
                output, (x,y) = self.detect_pick_up(frame)

            self.update_setpoint(x,y)

            cv2.imshow("output", output) #np.hstack([frame, output])) #np.hstack([frame, output]))
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    def detect_apriltag(self, frame):
        fx = 100  # TO DO: focal length x (in pixels)
        fy = 100  # TO DO: focal length y (in pixels)
        cx = 0  # principal point x(in pixels)
        cy = 0  # principal point y(in pixels)
        TS = 0.1459  # tag side length (in meters)

        gray = cv2.cvtColor(frame, cv2.COLOR_RGB2GRAY)
        detections, dimg = self.detector.detect(gray, return_image=True)
        overlay = frame // 2 + dimg[:, :, None] // 2

        num_detections = len(detections)
        #return overlay, (-1,-1)
        relevant_tags_seen = 0

        if num_detections >0:
            pixel_loc = []
            for i, tags in enumerate(detections):
                print(tags[1])
                if self.HOME_BASE_FLAG:
                    if tags[1] in [119, 120, 121, 122]: #== 119 or tags[1] == 120 or tags[1] == 121 or tags[1] == 122:
                        break
                elif self.FERRY_FLAG:
                    if tags[1] in [123, 124, 125, 126]: #== 123 or tags[1] == 124 or tags[1] == 125 or tags[1] == 126:
                        break
                elif self.DROP_OFF_FLAG:
                    if tags[1] in [127, 128, 129, 130]: #== 127 or tags[1] == 128 or tags[1] == 129 or tags[1] == 130:
                        break
                elif self.PICK_UP_FLAG:
                    if tags[1] in [131, 132, 133, 134]: #== 131 or tags[1] == 132 or tags[1] == 133 or tags[1] == 134:
                        break
                else:
                    return frame, (-1,-1)

            pixel_loc.append(tags.center)
            #centroidX = 0
            #centroidY = 0
            #for i in range(0, relevant_tags_seen):
            centroidX = int(pixel_loc[0][0])
            centroidY = int(pixel_loc[0][1])
            cv2.rectangle(overlay, (centroidX - 5, centroidY - 5), (centroidX + 5, centroidY + 5), (0, 128, 255), -1)
            return overlay, (centroidX, centroidY)

        return frame, (-1,-1)

    def detect_pick_up(self, frame):

        x = -1
        y = -1

        ratio = 1
        resolution = 0.001*25.4/60.0
        alt = 1.2
        f_length = 5.0*0.001

        resized = imutils.resize(frame, width=300)
        ratio = frame.shape[0] / float(resized.shape[0])

        gray = cv2.cvtColor(resized, cv2.COLOR_BGR2GRAY)
        blurred = cv2.GaussianBlur(gray, (3, 3), 0)

        thresh = cv2.threshold(blurred, 100, 255, cv2.THRESH_BINARY_INV)[1]
        #return thresh, (x,y)
        cnts = cv2.findContours(thresh.copy(), cv2.RETR_EXTERNAL,
    	   cv2.CHAIN_APPROX_SIMPLE)
        cnts = imutils.grab_contours(cnts)
        sd = ShapeDetector()

        for c in cnts:
    	# compute the center of the contour, then detect the name of the
    	# shape using only the contour

            shape = sd.detect(c)

            if shape == 'square':
        	# multiply the contour (x, y)-coordinates by the resize ratio,
        	# then draw the contours and the name of the shape on the image
                M = cv2.moments(c)
                cX = int((M["m10"] / M["m00"]) * ratio)
                cY = int((M["m01"] / M["m00"]) * ratio)
                peri = cv2.arcLength(c, True)
                print(peri)
                c = c.astype("float")
                c *= ratio
                c = c.astype("int")
                cv2.drawContours(frame, [c], -1, (0, 255, 0), 2)
                cv2.circle(frame, (cX, cY), 3, (255, 255, 255), -1)
                """
                side = resolution*peri/4.0;
                side_world = alt*side/f_length

                if side_world < 10.0:
                    continue
                else:
                    print("HERE")
                    c = c.astype("float")
                    c *= ratio
                    c = c.astype("int")
                    cv2.drawContours(frame, [c], -1, (0, 255, 0), 2)
                    cv2.circle(frame, (cX, cY), 3, (255, 255, 255), -1)
                """
            else:
                continue

        return frame, (x,y)

    def detect_drop_off(self, frame):

        blur = cv2.medianBlur(frame,5)
        gray = cv2.cvtColor(blur, cv2.COLOR_BGR2GRAY)
        output = frame.copy()
        circles = cv2.HoughCircles(gray,cv2.HOUGH_GRADIENT,1.2,100)
        x = -1
        y = -1
        # ensure at least some circles were found
        if circles is not None:
        	# convert the (x, y) coordinates and radius of the circles to integers
        	circles = np.round(circles[0, :]).astype("int")

        	# loop over the (x, y) coordinates and radius of the circles
        	for (x, y, r) in circles:
        		# draw the circle in the output image, then draw a rectangle
        		# corresponding to the center of the circle
                #print(f"{x}, {y}")
        		cv2.circle(output, (x, y), r, (0, 255, 0), 4)
        		cv2.rectangle(output, (x - 5, y - 5), (x + 5, y + 5), (0, 128, 255), -1)

        return output, (x,y)

    def detect_border(self, frame):

        ilowH = 21
        ihighH = 39
        ilowS = 157
        ihighS = 255
        ilowV = 111
        ihighV = 210
        x = -1
        y = -1
        width = self.width
        height = self.height

        hsv = cv2.cvtColor(frame,cv2.COLOR_BGR2HSV)

        lower_hsv = np.array([ilowH, ilowS, ilowV])
        higher_hsv = np.array([ihighH, ihighS, ihighV])
        mask = cv2.inRange(hsv,lower_hsv,higher_hsv)

        location = cv2.findNonZero(mask)

        if location is not None:
            #total = location[0] * location[1]
            try:
                line = cv2.fitLine(location, cv2.DIST_L2,0,0.01,0.01)
                slope = line[1]/line[0] ;
                y_intercept = line[3] - slope * line[2];
                x_intercept = -1*(y_intercept/slope);
                y1 = slope*0+y_intercept
                y2 = slope*width+y_intercept
                cv2.line(frame,(0,y1),(width,y2),(255,0,0),5)

                slope_perp = -1/slope
                y_int_perp = height/2-slope_perp*width/2
                x_perp = int((y_int_perp-y_intercept) / (slope-slope_perp))
                y_perp = int(slope_perp*x_perp + y_int_perp)
                cv2.line(frame,(int(width/2),int(height/2)),(x_perp,y_perp),(255,0,0),5)
                cv2.rectangle(frame, (x_perp - 5, y_perp - 5), (x_perp + 5, y_perp + 5), (0, 128, 255), -1)
                return frame, (x_perp,y_perp)
            except (OverflowError, ValueError):
                return frame, (x,y)
        else:
            return frame, (x,y)

def check_flags(FLAG_ARRAY):
    print(FLAG_ARRAY)

def main():

    HOME_BASE_FLAG = False
    FERRY_FLAG = False
    DROP_OFF_FLAG = True
    PICK_UP_FLAG = False
    BORDER_FLAG = False
    FLAG_ARRAY = [HOME_BASE_FLAG, FERRY_FLAG, DROP_OFF_FLAG, PICK_UP_FLAG, BORDER_FLAG]

    img_proc = ImageProcessing(FLAG_ARRAY)
    img_proc.print_flags()
    img_proc.process_images()


if __name__=='__main__':
    main()
