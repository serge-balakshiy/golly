from glife import *     # for rect class
import golly as g

r = rect( g.getselrect() )

if r.empty:
   g.warn("There is no selection.")

else:
   for row in range (r.top, r.top + r.height):
      for col in range (r.left, r.left + r.width):
         g.setcell(col, row, 1 - g.getcell(col, row))

   # only fit selection if it's not completely visible in viewport???
   # ie. if not r.visible: g.fitsel()
   g.fitsel()
