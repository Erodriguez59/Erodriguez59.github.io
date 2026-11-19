// All button
document.getElementById("allBtn").addEventListener("click", () => {

    document.querySelectorAll(".project-card").forEach(card => {

        card.style.display = "";

    });

});

// Top 5 button
document.getElementById("topBtn").addEventListener("click", () => {

    document.querySelectorAll(".project-card").forEach(card => {

        if (card.classList.contains("top5")) {
            card.style.display = "";
        }
        else {
            card.style.display = "none";
        }

    });

});

// Most recent button
document.getElementById("recentBtn").addEventListener("click", () => {

    document.querySelectorAll(".project-card").forEach(card => {

        if (card.classList.contains("recent")) {
            card.style.display = "";
        }
        else {
            card.style.display = "none";
        }

    });

});

const createProjectBtn = document.getElementById("createProjectBtn");
const creationModal = document.getElementById("creationModal");
const closeCreationBtn = document.getElementById("closeCreationBtn");

createProjectBtn.addEventListener("click", () => {
    creationModal.style.display = "block";
});

closeCreationBtn.addEventListener("click", () => {
    creationModal.style.display = "none";
});

const projectDetailModal = document.getElementById("projectDetailModal");
const closeDetailBtn = document.getElementById("closeDetailBtn");
const projectDetailContent = document.getElementById("projectDetailContent");

const projectDetails = {
    ultima: `
        <h1>ULTIMA2.0</h1>

        <h2>Overview</h2>
        <p>Secure IPC operating systems project built in C++.</p>

        <h2>Objectives</h2>
        <ul>
            <li>Authentication</li>
            <li>Permission checks</li>
            <li>Message validation</li>
            <li>Secure receiving</li>
        </ul>

        <h2>Technologies</h2>
        <ul>
            <li>C++</li>
            <li>Linux</li>
            <li>pthreads</li>
            <li>Semaphores</li>
        </ul>

        <h2>Challenges</h2>
        <p>Handling permission checks, message integrity, and safe message deletion.</p>

        <h2>Future Improvements</h2>
        <p>Encryption, role-based access, and stronger logging.</p>

        <a href="https://github.com/Erodriguez59/CSCI-C435-ULTIMA2.0-" target="_blank" class="github-btn">
            GitHub Repository
        </a>
    `,

    portfolio: `
        <h1>Portfolio Website</h1>

        <h2>Overview</h2>
        <p>A personal website used to archive projects, blog posts, and development progress.</p>

        <h2>Objectives</h2>
        <ul>
            <li>Build a project gallery</li>
            <li>Create reusable project cards</li>
            <li>Prepare for GitHub Pages deployment</li>
        </ul>

        <h2>Technologies</h2>
        <ul>
            <li>HTML</li>
            <li>CSS</li>
            <li>JavaScript</li>
            <li>GitHub Pages</li>
        </ul>

        <h2>Future Improvements</h2>
        <p>Add backend storage, admin controls, and automatic project creation.</p>

        <a href="https://github.com/Erodriguez59/Erodriguez59.github.io" target="_blank" class="github-btn">
            GitHub Repository
        </a>
    `
};

document.querySelectorAll(".learn-more-btn").forEach(button => {
    button.addEventListener("click", () => {
        const projectName = button.dataset.project;
        projectDetailContent.innerHTML = projectDetails[projectName];
        projectDetailModal.style.display = "block";
    });
});

closeDetailBtn.addEventListener("click", () => {
    projectDetailModal.style.display = "none";
});

