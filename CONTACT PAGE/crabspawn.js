// JavaScript source code
<script>
    const footer = document.querySelector("footer");
    const originalCrab = document.querySelector(".crab");

    originalCrab.addEventListener("click", spawnCrab);

    function spawnCrab() {
        const newCrab = originalCrab.cloneNode(true);

    newCrab.classList.add("baby-crab");

    newCrab.addEventListener("click", spawnCrab);

    footer.appendChild(newCrab);

        newCrab.addEventListener("animationend", (event) => {
            if (event.animationName === "babyCrabWalk") {
        newCrab.remove();
            }
        });
    }
</script>