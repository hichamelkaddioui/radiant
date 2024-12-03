let pixelSize = 10;

/**
 * Canvas drawing
 */

// Get the current state of the canvas as a bitmap
const getCanvasState = (pixelSize) => {
  const canvas = document.getElementById("canvas");
  const ctx = canvas.getContext("2d");
  const imageData = ctx.getImageData(0, 0, canvas.width, canvas.height);
  const canvasState = [];
  for (let y = 0; y < canvas.height; y += pixelSize) {
    const row = [];
    for (let x = 0; x < canvas.width; x += pixelSize) {
      const idx = (y * canvas.width + x) * 4;
      const r = imageData.data[idx];
      const g = imageData.data[idx + 1];
      const b = imageData.data[idx + 2];

      row.push(r === 0 && g === 0 && b === 0 ? 0 : 1);
    }
    canvasState.push(row);
  }
  return canvasState;
};

// Draw the canvas based on the state array
const drawCanvas = (state, pixelSize) => {
  const canvas = document.getElementById("canvas");
  const ctx = canvas.getContext("2d");
  const width = canvas.width;
  const height = canvas.height;

  ctx.clearRect(0, 0, width, height);
  for (let y = 0; y < height / pixelSize; y++) {
    for (let x = 0; x < width / pixelSize; x++) {
      ctx.fillStyle = state[y][x] === 0 ? "black" : "white";
      ctx.fillRect(x * pixelSize, y * pixelSize, pixelSize, pixelSize);
    }
  }

  outputCanvasState(state);
};

// Draw the canvas based on the state array
const outputCanvasState = (canvasState) => {
  const outputArray = canvasState.map((row) => row.join(""));
  const cppOutputArray = outputArray.map((l) =>
    l
      .split("")
      .map((c, index) => {
        if (index % 8 === 0) {
          return `0b${c}`;
        }

        if (index % 8 === 7) {
          return `${c},`;
        }

        return c;
      })
      .join("")
  );
  document.getElementById("bitmapInput").value = outputArray.join("\n");
  document.getElementById("outputCpp").textContent = cppOutputArray.join("\n");
};

/**
 * Load bitmap from PNG
 */

// Function to decode and convert the base64 image to a bitmap
const decodeAndConvert = (base64Data) => {
  const img = new Image();
  return new Promise((resolve, reject) => {
    img.onload = () => {
      const canvas = document.createElement("canvas");
      const ctx = canvas.getContext("2d");

      canvas.width = 120;
      canvas.height = 32;
      ctx.drawImage(img, 0, 0, 120, 32);

      const imageData = ctx.getImageData(0, 0, 120, 32);
      const pixelMap = [];
      for (let y = 0; y < 32; y++) {
        const row = [];
        for (let x = 0; x < 120; x++) {
          const idx = (y * 120 + x) * 4;
          const r = imageData.data[idx];
          const g = imageData.data[idx + 1];
          const b = imageData.data[idx + 2];

          const gray = Math.round(0.299 * r + 0.587 * g + 0.114 * b);
          row.push(gray === 0 ? 0 : 1);
        }
        pixelMap.push(row);
      }

      resolve(pixelMap);
    };

    img.onerror = () => {
      reject(new Error("Failed to load image"));
    };

    img.src = base64Data;
  });
};

// Handle file upload, convert PNG to bitmap, and display it
const handleImageUpload = async (pixelSize) => {
  const fileInput = document.getElementById("fileInput");
  const file = fileInput.files[0];
  if (!file) {
    alert("Please select an image.");
    return;
  }

  const reader = new FileReader();
  reader.onload = async (e) => {
    const base64Data = e.target.result;
    try {
      const state = await decodeAndConvert(base64Data);
      drawCanvas(state, pixelSize);
    } catch (error) {
      console.error("Error decoding and converting the image:", error);
    }
  };
  reader.readAsDataURL(file);
};

/**
 * Load bitmap from textarea
 */
const loadBitmap = (width, height, pixelSize) => {
  const bitmapInput = document.getElementById("bitmapInput").value.trim();
  const state = bitmapInput
    .split("\n")
    .map((row) => row.trim().split("").map(Number));

  if (
    state.length !== height / pixelSize ||
    state.some((row) => row.length !== width / pixelSize)
  ) {
    alert("Bitmap size does not match canvas size!");
    return;
  }

  drawCanvas(state, pixelSize);
};

/**
 * Main
 */
const initCanvas = () => {
  // Create, scale and draw the canvas
  const canvas = document.getElementById("canvas");

  canvas.width *= pixelSize;
  canvas.height *= pixelSize;

  const width = canvas.width;
  const height = canvas.height;
  const initState = Array.from({ length: height / pixelSize }, () =>
    Array(width / pixelSize).fill(0)
  );

  drawCanvas(initState, pixelSize);

  // Pencil and Eraser
  let currentTool = "pencil";
  let isMouseDown = false;

  canvas.addEventListener("mousedown", (e) => {
    isMouseDown = true;
    handleCanvasClick(e);
  });

  canvas.addEventListener("mousemove", (e) => {
    if (isMouseDown) {
      handleCanvasClick(e);
    }
  });

  document.addEventListener("mouseup", () => {
    isMouseDown = false;
  });

  const handleCanvasClick = (e) => {
    const x = Math.floor(e.offsetX / pixelSize);
    const y = Math.floor(e.offsetY / pixelSize);
    const state = getCanvasState(pixelSize);

    if (currentTool === "pencil") {
      state[y][x] = 1;
    } else if (currentTool === "eraser") {
      state[y][x] = 0;
    }

    drawCanvas(state, pixelSize);
  };

  document.getElementById("pencilTool").addEventListener("click", () => {
    currentTool = "pencil";
  });

  document.getElementById("eraserTool").addEventListener("click", () => {
    currentTool = "eraser";
  });

  // Image upload
  document.getElementById("imageUploadForm").addEventListener("submit", (e) => {
    e.preventDefault();
    handleImageUpload(pixelSize);
  });

  // Load bitmap
  document.getElementById("loadBitmap").addEventListener("click", () => {
    loadBitmap(width, height, pixelSize);
  });
};

document.addEventListener("DOMContentLoaded", initCanvas);
