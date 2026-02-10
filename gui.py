#Immense sh@t GUI that can break, very early version due to not needing a gui at the first place

import tkinter as tk
from tkinter import filedialog, messagebox
from PIL import Image, ImageTk
import subprocess
import os
import sys
import threading

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
#On WINDOWS keep as gaussian_blur.exe on LINUX remove .exe
GAUSSIAN_CMD = os.path.join(BASE_DIR, "gaussian_blur.exe")



if not os.path.exists(GAUSSIAN_CMD):
    messagebox.showerror("Error", f"gaussian_blur not found:\n{GAUSSIAN_CMD}")
    sys.exit(1)


class GaussianGUI:
    def __init__(self, root):
        self.root = root
        self.root.title("Gaussian Blur")
        self.root.geometry("520x720")
        self.root.resizable(False, False)

        self.input_path = None
        self.current_image_path = None
        self.tk_img = None

        tk.Label(root, text="Gaussian Blur", font=("Arial", 20)).pack(pady=10)

        self.canvas = tk.Canvas(
            root, width=400, height=300,
            bg="black", highlightthickness=1
        )
        self.canvas.pack(pady=10)
        self.canvas.bind("<Configure>", self.on_canvas_resize)

        controls = tk.Frame(root)
        controls.pack(fill="x", pady=10)

        self.lock = tk.BooleanVar(value=True)
        tk.Checkbutton(
            controls,
            text="Lock kernel to sigma (kernel = 6σ + 1)",
            variable=self.lock
        ).pack()

        self.sigma = tk.DoubleVar(value=1.0)
        tk.Scale(
            controls, from_=0.5, to=5.0, resolution=0.1,
            orient="horizontal", label="Sigma",
            variable=self.sigma, command=self.on_slider
        ).pack(fill="x", padx=20)

        self.kernel = tk.IntVar(value=7)
        tk.Scale(
            controls, from_=3, to=31, resolution=2,
            orient="horizontal", label="Kernel Size",
            variable=self.kernel, command=self.on_slider
        ).pack(fill="x", padx=20)

        buttons = tk.Frame(root)
        buttons.pack(pady=15)

        tk.Button(buttons, text="Open Image", width=15,
                  command=self.open_image).pack(side="left", padx=10)

        tk.Button(buttons, text="Apply Blur", width=15,
                  command=self.apply_blur).pack(side="left", padx=10)

        self.loading = tk.Toplevel(root)
        self.loading.withdraw()
        self.loading.overrideredirect(True)
        self.loading.geometry("200x100")
        tk.Label(self.loading, text="Processing...\nPlease wait",
                 font=("Arial", 12)).pack(expand=True)


    def show_loading(self):
        self.loading.update_idletasks()
        x = self.root.winfo_x() + 160
        y = self.root.winfo_y() + 300
        self.loading.geometry(f"+{x}+{y}")
        self.loading.deiconify()
        self.loading.grab_set()

    def hide_loading(self):
        self.loading.grab_release()
        self.loading.withdraw()


    def open_image(self):
        path = filedialog.askopenfilename(
            filetypes=[("Images", "*.png *.jpg *.jpeg *.bmp *.tga")]
        )
        if not path:
            return
        self.input_path = os.path.abspath(path)
        self.show_image(self.input_path)

    def on_slider(self, _):
        if self.lock.get():
            k = int(6 * self.sigma.get() + 1)
            if k % 2 == 0:
                k += 1
            self.kernel.set(max(3, min(k, 31)))

    def apply_blur(self):
        if not self.input_path:
            messagebox.showerror("Error", "Open an image first.")
            return

        output_path = filedialog.asksaveasfilename(
            defaultextension=".png",
            filetypes=[("PNG", "*.png"), ("JPEG", "*.jpg"), ("All files", "*.*")]
        )
        if not output_path:
            return

        self.show_loading()

        threading.Thread(
            target=self.run_blur,
            args=(output_path,),
            daemon=True
        ).start()

    def run_blur(self, output_path):
        try:
            subprocess.run(
                [
                    GAUSSIAN_CMD,
                    self.input_path,
                    os.path.abspath(output_path),
                    str(self.kernel.get()),
                    str(self.sigma.get())
                ],
                cwd=BASE_DIR,
                check=True
            )
            self.root.after(0, lambda: self.finish_blur(output_path))
        except Exception as e:
            self.root.after(0, lambda: self.error_blur(e))

    def finish_blur(self, output_path):
        self.hide_loading()
        self.show_image(output_path)

    def error_blur(self, error):
        self.hide_loading()
        messagebox.showerror("Error", str(error))

    def show_image(self, path):
        self.current_image_path = path
        img = Image.open(path)
        img.thumbnail((400, 300))
        self.tk_img = ImageTk.PhotoImage(img)
        self.canvas.delete("all")
        self.canvas.create_image(200, 150, image=self.tk_img)

    def on_canvas_resize(self, _):
        if self.current_image_path:
            self.show_image(self.current_image_path)


if __name__ == "__main__":
    root = tk.Tk()
    GaussianGUI(root)
    root.mainloop()
