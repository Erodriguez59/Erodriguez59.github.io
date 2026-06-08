# THE RODRIGUEZ PROJECT BUG REPORTING
>BY ERIK RODRIGUEZ
>
 ## THE UI
### [FIXED] [UI] #001 Blog Page Background not continuing after so many posts. (BOLD THIS)
**Cause:** Paint splashes are positioned using percentages of the page during page load.

**Fix:** Swapped to a repeating pattern that continues as the page grows.
 
 ## THE CSS
### [FIXED] [CSS] #001 Speech bubble tails did not match bubble colors.
**Cause:** Tail background hardcoded to pink.

**Fix:** Changed tail background to inherit parent color.

 ## THE JAVASCRIPT
 ### [FIXED] [JS] #001 Speech bubbles always appear on left side.
   **Cause:** Random position generated only once during page load.
   
   **Fix:** Moved random position generation into addBtn event.

 ## THE STORAGE

 ## THE NAVIGATION

 ## THE CONTENT

 ## THE PERFORMANCE
