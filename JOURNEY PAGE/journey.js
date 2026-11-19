function drawJourneyArrows() {
    const svg = document.querySelector(".journey-lines");

    svg.querySelectorAll(".journey-arrow")
        .forEach(path => path.remove());

    const width = document.documentElement.scrollWidth;
    const height = document.documentElement.scrollHeight;

    svg.setAttribute("viewBox", `0 0 ${width} ${height}`);

    const connections = [
        [".LOGIC", ".LBS"],
        [".LBS", ".FDSA"],
        [".FDSA", ".BOT"],
        [".BOT", ".DCTI"]
    ];

    const mobileLayout = window.innerWidth <= 900;

    connections.forEach(([startSelector, endSelector]) => {

        const startBox = document.querySelector(startSelector);
        const endBox = document.querySelector(endSelector);

        const start = startBox.getBoundingClientRect();
        const end = endBox.getBoundingClientRect();

        const scrollX = window.scrollX;
        const scrollY = window.scrollY;

        let startX;
        let endX;

        const startY = start.bottom + scrollY;
        const endY = end.top + scrollY;

        if (mobileLayout) {

            // Straight down between centered boxes
            startX =
                start.left +
                scrollX +
                start.width / 2;

            endX =
                end.left +
                scrollX +
                end.width / 2;

        } else {

            const goingRight = end.left > start.left;

            const edgeOffset = 60;

            if (goingRight) {

                startX =
                    start.right +
                    scrollX -
                    edgeOffset;

                endX =
                    end.left +
                    scrollX +
                    edgeOffset;

            } else {

                startX =
                    start.left +
                    scrollX +
                    edgeOffset;

                endX =
                    end.right +
                    scrollX -
                    edgeOffset;
            }
        }

        const verticalGap = endY - startY;

        const curveAmount =
            Math.max(35, verticalGap * 0.4);

        const path =
            document.createElementNS(
                "http://www.w3.org/2000/svg",
                "path"
            );

        path.classList.add("journey-arrow");

        if (mobileLayout) {

            path.setAttribute(
                "d",
                `
                M ${startX} ${startY}
                L ${endX} ${endY}
                `
            );

        } else {

            path.setAttribute(
                "d",
                `
                M ${startX} ${startY}
                C ${startX} ${startY + curveAmount},
                  ${endX} ${endY - curveAmount},
                  ${endX} ${endY}
                `
            );
        }

        path.setAttribute(
            "marker-end",
            "url(#arrowhead)"
        );

        svg.appendChild(path);
    });
}

window.addEventListener(
    "load",
    drawJourneyArrows
);

window.addEventListener(
    "resize",
    drawJourneyArrows
);
