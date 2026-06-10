// JavaScript source code
const input = document.getElementById("userInput");
const addBtn = document.getElementById("addBtn");
const postContainer = document.getElementById("postContainer");
const ADMIN_PASSWORD = "salamander";

function checkPassword(actionName) {
    const password = prompt(`Enter password to ${actionName}:`);

    if (password !== ADMIN_PASSWORD) {
        alert("Wrong password. Action cancelled.");
        return false;
    }

    return true;
}

let posts = JSON.parse(localStorage.getItem("posts")) || [];

let visibleCount = 10;

function savePosts() {
    localStorage.setItem("posts", JSON.stringify(posts));
}

function convertLinks(text) {
    const urlPattern = /(https?:\/\/[^\s]+)/g;

    return text.replace(
        urlPattern,
        '<a href="$1" target="_blank">$1</a>'
    );
}

function renderPosts() {
    postContainer.innerHTML = "";

    const visiblePosts = posts.slice(0, visibleCount);

    visiblePosts.forEach((post, index) => {
        const postBox = document.createElement("div");
        postBox.className =
            `post ${post.position || "center"} ${post.color || "pink"}`;

        postBox.innerHTML = `
        <div class="post-timestamp">
        ${post.createdAt}
        </div>
            
             <p>${convertLinks(post.text)}</p>

            <div class="post-buttons">
                <button onclick="editPost(${index})">Edit</button>
                <button onclick="deletePost(${index})">Delete</button>
            </div>
        `;

        postContainer.appendChild(postBox);
    });
}

addBtn.addEventListener("click", () => {
    const text = input.value.trim();

    if (text === "") return;

    const positions = ["left", "center", "right"];
    const randomPosition = positions[Math.floor(Math.random() * positions.length)];

    const colors = [
        "pink",
        "peach",
        "lavender",
        "mint",
        "sky",
        "cream"
    ];

    const randomColor = colors[Math.floor(Math.random() * colors.length)];

    posts.unshift({
        text: text,
        position: randomPosition,
        color: randomColor,
        createdAt: new Date().toLocaleString()
    });

    input.value = "";
    savePosts();
    renderPosts();
});

function deletePost(index) {
    if (!checkPassword("delete this post")) return;

    const confirmDelete = confirm("Password correct. Are you sure you want to delete this post?");

    if (!confirmDelete) return;

    posts.splice(index, 1);
    savePosts();
    renderPosts();
}

function editPost(index) {
    if (!checkPassword("edit this post")) return;

    const newText = prompt("Edit your post:", posts[index].text);

    if (newText !== null && newText.trim() !== "") {
        posts[index].text = newText.trim();
        savePosts();
        renderPosts();
    }
}

window.addEventListener("scroll", () => {
    const nearBottom =
        window.innerHeight + window.scrollY >= document.body.offsetHeight - 100;

    if (nearBottom && visibleCount < posts.length) {
        visibleCount += 5;
        renderPosts();
    }
});

renderPosts();