// JavaScript source code for transition
document.querySelectorAll("a").forEach(link => {
    link.addEventListener("click", function (e) {
        e.preventDefault();

        const href = this.href;

        document.body.classList.add("fade-out");

        setTimeout(() => {
            window.location.href = href;
        }, 500);
    });
});