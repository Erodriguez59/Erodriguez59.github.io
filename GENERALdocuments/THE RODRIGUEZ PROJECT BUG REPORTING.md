# THE RODRIGUEZ PROJECT BUG REPORTING
>BY ERIK RODRIGUEZ
>
 ## THE UI
### [FIXED] [P3] [UI] #001 Blog Page Background not continuing after so many posts.
**Cause:** Paint splashes are positioned using percentages of the page during page load.

**Fix:** Swapped to a repeating pattern that continues as the page grows.

### [WON'T FIX] [P1] [UI] #002 Journey Page Arrows are fucked.
**Issue:** The SVG arrows break when on different sized page resolutions.
**Reason:** This currently isn't that big of an issue as the page isn't online yet.

 ### [OPEN] [P4] [UI] #003 The Wonder Blog Return to Landing button is out of place.
 **Cause:** [TBD]

 **Fix:** [TBD]
 
 ## THE CSS
### [FIXED] [P4] [CSS] #001 Speech bubble tails did not match bubble colors.
**Cause:** Tail background hardcoded to pink.

**Fix:** Changed tail background to inherit parent color.

### [FIXED] [P2] [CSS] #002 Goals text barely visible.
**Cause:** The text is barely visible due to the flexbox backgrounds and font-color.

**Fix:** We neon-signed some of the less readable portions.

### [FIXED] [P4] [CSS] #003 Crab animation feels off.
**Cause:** The animation for the crab doesn't seem animated enough.

**Fix:** We caused the crab to intermittently stop moving and wiggle around.

 ## THE JAVASCRIPT
 ### [FIXED] [P4] [JS] #001 Speech bubbles always appear on left side.
   **Cause:** Random position generated only once during page load.
   
   **Fix:** Moved random position generation into addBtn event.

### [FIXED] [P2] [JS] #002 Delete/Edit blog has no security?
**Cause:** With no security, anyone can delete/edit blog posts. This was due to a lack of forethought since it wasn't online yet.

**Fix:** I added a simple password authentication, nothing too fancy, nothing to secure. Just to show it could be done.

 ## THE STORAGE
 ### [FIXED] [P3] [STR] #001 Unorganized Github Repository
  **Cause:** Outdated version was messy since I was just throwing things in there as needed.

  **Fix:** I deleted all the outdated files, keeping only the index.html **AND** the README.md; then added the files of the current project.
  >Files were added on June 6th, 2026 at 1:55 PM EST.

 ## THE NAVIGATION
 ### [FIXED] [P1] [NAV] #001 Page not found.
 **Cause:** We were using direct pathing, while the directory was that of my IU workstation rather than my home PC.

 **Fix:** We swapped to using relative pathing and that fixed our issues with pages not being found.

 ### [IN PROGRESS] [P4] [NAV] #002 Transitions work one way.
 **Issue:** Transitions work going through the (what I like to call the tree) directory to the branches, but do not work when going back to the root.

**Current Solution:** 

body {
    
    opacity: 1;
    
    transition: opacity 0.5s ease;
    
}

body.fade-out {
    opacity: 0;
}
**AND**
getting the right amount of "../" + "transition.js"

 **Reason:** As long as they work going through the branches I am okay with that.
 ## THE CONTENT
 ### [FIXED] [P3] [CON] #001 Images not found.
 **Cause:** We were using direct pathing, while the directory was that of my IU workstation rather than my home PC. **PLUS** we had a folder within each page that held the photos used by that page.

 **Fix:** We managed to get relative pathing done for each pages images to a GENERALimage folder where I now store all the images used.
 
 ## THE PERFORMANCE
