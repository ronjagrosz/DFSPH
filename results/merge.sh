# -r = framerate, -f = format, -s = resolution
# -i = image directory, %04d makes 0001 to 9999
# -crf = quality, 15-25 is good

ffmpeg -r 15 -f image2 -s 640x480 -i frames/frame%04d.png -vcodec libx264 -crf 25  -pix_fmt yuv420p out.avi

