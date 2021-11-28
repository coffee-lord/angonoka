import { render } from "@testing-library/react";
import { TaskStrip } from "./TaskStrip";

test("displays task name", () => {
  const { queryByText } = render(
    <TaskStrip name="Task Name" width={1} offset={0} />
  );

  expect(queryByText("Task Name")).toBeTruthy();
});

test("has correct with and offset", () => {
  const { getByRole } = render(
    <TaskStrip name="Task Name" width={0.25} offset={0.5} />
  );

  const strip = getByRole("link");

  expect(strip.style.width).toEqual("25%");
  expect(strip.style.left).toEqual("50%");
});
