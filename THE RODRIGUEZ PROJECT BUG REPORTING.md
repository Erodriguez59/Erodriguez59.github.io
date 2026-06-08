# THE RODRIGUEZ PROJECT BUG REPORTING
>BY ERIK RODRIGUEZ
>
 ## THE UI
### [FIXED] [P2] [UI] #001 Blog Page Background not continuing after so many posts.
**Cause:** Paint splashes are positioned using percentages of the page during page load.

**Fix:** Swapped to a repeating pattern that continues as the page grows.

### [WON'T FIX] [P1] [UI] #002 Journey Page Arrows are fucked.
**Issue:** The SVG arrows break when on different sized page resolutions.
**Reason:** This currently isn't that big of an issue as the page isn't online yet.
 
 ## THE CSS
### [FIXED] [P1] [CSS] #001 Speech bubble tails did not match bubble colors.
**Cause:** Tail background hardcoded to pink.

**Fix:** Changed tail background to inherit parent color.

 ## THE JAVASCRIPT
 ### [FIXED] [P1] [JS] #001 Speech bubbles always appear on left side.
   **Cause:** Random position generated only once during page load.
   
   **Fix:** Moved random position generation into addBtn event.

 ## THE STORAGE

 ## THE NAVIGATION
 ### [FIXED] [P3] [NAV] #001 Page not found.
 **Cause:** We were using direct pathing, while the directory was that of my IU workstation rather than my home PC.

 **Fix:** We swapped to using relative pathing and that fixed our issues with pages not being found.
 
 ## THE CONTENT

 ## THE PERFORMANCE
