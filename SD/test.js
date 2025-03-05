function testDarkModeToggle() {
  // Get the toggle and body elements
  const toggle = document.getElementById('darkModeToggle');
  const body = document.body;

  // Test initial state (light mode)
  if (localStorage.getItem('isDarkMode') !== 'true') {
    body.classList.remove('dark-mode');
  }

  // Simulate user clicking the toggle
  toggle.click();

  // Test if dark mode is applied
  if (body.classList.contains('dark-mode')) {
      console.log('Dark mode applied correctly.');
  } else {
    console.error('Dark mode not applied!');
  }

  // Test if state is saved in localStorage
  if (localStorage.getItem('isDarkMode') === 'true') {
    console.log('State correctly saved to localStorage.');
  } else {
    console.error('State not saved to localStorage!');
  }

    // Test if the button toggle is working
    if(document.getElementById('darkModeToggle').checked){
        console.log("Dark mode button is checked");
    }
}
