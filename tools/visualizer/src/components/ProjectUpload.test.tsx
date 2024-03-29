import { render, fireEvent } from "@testing-library/react";
import { ProjectUpload } from "./ProjectUpload";

test("has text", () => {
  const { getByText } = render(<ProjectUpload onUpload={() => {}} />);

  const button = getByText("Upload");
  expect(button.nodeName).toEqual("BUTTON");
  expect(button).toBeEnabled();
});

test("callback", (done) => {
  const callback = (data: any) => {
    expect(data).toEqual({ hello: "world" });
    done();
  };
  const { container, getByText } = render(
    <ProjectUpload onUpload={callback} />
  );

  const input = container.getElementsByTagName("INPUT")[0];
  expect(input).toBeInTheDocument();

  fireEvent.click(getByText("Upload"));

  const file = {
    text: async () => '{"hello":"world"}',
  };
  fireEvent.change(input, {
    target: { files: [file] },
  });
  fireEvent.input(input);
});
