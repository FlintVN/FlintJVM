
export class Semaphore {
    private tasks: (() => void)[] = [];
    private counter: number;

    public constructor(count: number) {
        this.counter = count;
    }

    public acquire(): Thenable<void> {
        return new Promise<void>((resolve) => {
            const task = () => {
                this.counter--;
                resolve();
            };
            if(this.counter > 0)
                task();
            else
                this.tasks.push(task);
        });
    }

    public release() {
        this.counter++;
        if (this.tasks.length > 0) {
            const task = this.tasks.shift();
            if(task)
                task();
        }
    }
}
